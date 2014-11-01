/*
 * Copyright 2011-2013 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#include "common.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <bgfx.h>
#include <bx/timer.h>
#include "entry/entry.h"
#include "fpumath.h"
#include "json\json.h"
#include "pugixml.hpp"
#include <Box2D/Box2D.h>
#include "3rdparty\nanosvg\src\nanosvg.h"
#include "3rdparty\libtess2\Include\tesselator.h"

#include "stb_audio_mixer.h"
#include "stb_vorbis.h"
#include "stb.h"


extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "lunar.h"

#include <map>

#include <stdio.h>
#include <string.h>
#include "zlib.h"
#include "base64.h"
#include "spriter.h"
#include "wav.h"

float camShake = 0.f;
float camShakeDecay = 10.f;

TiledMap* LoadTMX( const char *filename )
{
	static std::map<std::string,struct TiledMap*> cache;

	std::map<std::string,struct TiledMap*>::iterator f = cache.find( filename );
	if ( f != cache.end() )
	{
		return f->second;
	}

	pugi::xml_document doc;
	if ( doc.load_file( filename ) )
	{
		TiledMap *tm = new TiledMap;
		Serialise( doc.child("map"), *tm );
		cache[filename] = tm;
		return tm;
	}

	return NULL;
}

bool FindTiledEntity( float pos[2], const char *filename, const char *filter, const char *ent )
{
	struct TiledMap* tm = LoadTMX(filename);
	for ( unsigned int i=0; i<tm->objectgroups.size(); i++ )
	{
		const TiledObjectGroup &group = tm->objectgroups[i];
		if ( filter && strstr(group.name.c_str(),filter)==NULL )
			continue;
		for (unsigned int j=0; j<group.objects.size(); j++)
		{
			const TiledObject &obj = group.objects[j];
			if ( obj.name != ent )
				continue;
			pos[0] = obj.x;
			pos[1] = (tm->height*tm->tileheight)-obj.y;
			return true;
		}
	}
	return false;
}


int emitPnt( std::vector<float> &pnts, int x, int y, float u, float v )
{
	int ret = pnts.size() / 4;
	pnts.push_back( (float)x );
	pnts.push_back( (float)y );
	pnts.push_back( u );
	pnts.push_back( v );
	return ret;
}

void CreateTiledScene( SpriterSceneNode *group, const char *filename, const char *filter )
{
	struct TiledMap* tm = LoadTMX(filename);

	for ( unsigned int i=0; i<tm->layers.size(); i++ )
	{
		TiledLayer const &tl = tm->layers[i];
		if ( filter && strstr(tl.name.c_str(),filter)==NULL )
			continue;
		if ( tl.datas.empty() )
			continue;
		TiledData const &td = tl.datas[0];
		std::vector<float> pnts;
		std::vector<unsigned short> indices;
		for (int y=0; y<tl.height; y++)
		{
			for (int x=0; x<tl.width; x++)
			{
				int index = (tl.height-1-y) * tl.width + x;
				if ( td.ids[index] == 0 )
					continue;
				int tileset = 0;
				TiledTileset const &ts = tm->tilesets[tileset];
				if ( ts.images.empty() )
					continue;
				TiledImage const &img = ts.images[0];
				float deltaU = (float)ts.tilewidth / img.width;
				float deltaV = (float)ts.tileheight / img.height;
				int numX = img.width / ts.tilewidth;
				//int numY = img.height / ts.tileheight;
				float halfPixelU = 0.f;//0.5f / img.width;
				float halfPixelV = 0.f;//0.5f / img.height;
				int tileid = td.ids[index] - ts.firstgid;
				int tx = tileid % numX;
				int ty = tileid / numX;
				int idx0 = emitPnt( pnts, x*tm->tilewidth, y*tm->tileheight, tx*deltaU+halfPixelU, ty*deltaV+deltaV+halfPixelV );
				int idx1 = emitPnt( pnts, x*tm->tilewidth+tm->tilewidth, y*tm->tileheight, tx*deltaU+deltaU+halfPixelU, ty*deltaV+deltaV+halfPixelV );
				int idx2 = emitPnt( pnts, x*tm->tilewidth+tm->tilewidth, y*tm->tileheight+tm->tileheight, tx*deltaU+deltaU+halfPixelU, ty*deltaV+halfPixelV );
				int idx3 = emitPnt( pnts, x*tm->tilewidth, y*tm->tileheight+tm->tileheight, tx*deltaU+halfPixelU, ty*deltaV+halfPixelV );
				indices.push_back( idx0 );
				indices.push_back( idx1 );
				indices.push_back( idx2 );
				indices.push_back( idx0 );
				indices.push_back( idx2 );
				indices.push_back( idx3 );
			}
		}

		SpriterSceneMesh *mesh = new SpriterSceneMesh(tm->tilesets[0].images[0].source.c_str(), &pnts[0], pnts.size()/4, &indices[0], indices.size() );
		group->AddChild(mesh);
	}
}

void CreateTiledBox2dCollider( b2World &world, void *ent, const char *filename, const char *filter )
{
	b2BodyDef groundBodyDef;
	b2Body* groundBody = world.CreateBody(&groundBodyDef);
	groundBody->SetUserData( ent );

	struct TiledMap* tm = LoadTMX(filename);

	for ( unsigned int i=0; i<tm->layers.size(); i++ )
	{
		TiledLayer const &tl = tm->layers[i];
		if ( filter && strstr(tl.name.c_str(),filter)==NULL )
			continue;
		if ( tl.datas.empty() )
			continue;
		TiledData const &td = tl.datas[0];
		std::vector<float> pnts;
		std::vector<unsigned short> indices;
		for (int y=0; y<tl.height; y++)
		{
			for (int x=0; x<tl.width; x++)
			{
				int index = (tl.height-1-y) * tl.width + x;
				if ( td.ids[index] == 0 )
					continue;
				if ( td.ids[index] != 2 )
					continue;

				float v[4][2] = { { x*tm->tilewidth, y*tm->tileheight },
							{ x*tm->tilewidth+tm->tilewidth, y*tm->tileheight },
							{ x*tm->tilewidth+tm->tilewidth, y*tm->tileheight+tm->tileheight },
							{ x*tm->tilewidth, y*tm->tileheight+tm->tileheight } };
				int ofs[4][2] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };

				for (int i=0; i<4; i++)
				{
					int nx = x + ofs[i][0];
					int ny = y + ofs[i][1];
					if ( nx < 0 || ny < 0 || nx >= tl.width || ny >= tl.height )
						continue;
					int nextindex = (tl.height-1-ny) * tl.width + nx;
					if ( td.ids[nextindex] == 2 )
						continue;
					b2EdgeShape groundShape;
					groundShape.Set( b2Vec2(v[i][0]*PixelsToMeters,v[i][1]*PixelsToMeters), b2Vec2(v[(i+1)%4][0]*PixelsToMeters,v[(i+1)%4][1]*PixelsToMeters) );

					// Add the ground fixture to the ground body.
					b2Fixture *groundFix = groundBody->CreateFixture(&groundShape, 0.0f);
					groundFix->SetUserData( "ground" );
				}
			}
		}
	}
}


enum EntityType
{
	ET_Level,
	ET_Rocket,
	ET_Enemy,
	ET_Player,
	ET_SpotEffect,
};

class WorldContacts;

struct SceneInfo
{
	b2World *world;
	SpriterSceneNode *scene;
	WorldContacts *contacts;
	std::vector<class Entity*> entities;
	std::vector<class Entity*> toadd;
};

class Entity
{
public:
	virtual ~Entity() {}
	virtual bool ShouldCollide( Entity *other ) { (void)other; return true; }
	virtual bool Update( SceneInfo *info, float dt ) { (void)info; (void)dt; return true; }
	virtual void OnContact( Entity *other, float dir[2] ) { (void)other; (void)dir; }
	virtual void OnDelete( SceneInfo *info ) { (void)info; }
	virtual EntityType Type() = 0;
};

class WorldContacts : public b2ContactListener
{
	std::map<std::string,int> contactCount;
public:
	virtual void BeginContact(b2Contact* contact) 
	{
		const char *namea = (const char*)contact->GetFixtureA()->GetUserData();
		const char *nameb = (const char*)contact->GetFixtureB()->GetUserData();
		std::string id;
		if ( strcmp( namea, nameb ) < 0 ) 
		{
			id = namea + std::string(":") + nameb;
		}
		else
		{
			id = nameb + std::string(":") + namea;
		}
		contactCount[id]++;
	}

	virtual void EndContact(b2Contact* contact) 
	{
		const char *namea = (const char*)contact->GetFixtureA()->GetUserData();
		const char *nameb = (const char*)contact->GetFixtureB()->GetUserData();
		std::string id;
		if ( strcmp( namea, nameb ) < 0 ) 
		{
			id = namea + std::string(":") + nameb;
		}
		else
		{
			id = nameb + std::string(":") + namea;
		}
		contactCount[id]--;
	}

	bool InContact( const char *namea, const char *nameb )
	{
		std::string id;
		if ( strcmp( namea, nameb ) < 0 ) 
		{
			id = namea + std::string(":") + nameb;
		}
		else
		{
			id = nameb + std::string(":") + namea;
		}
		return contactCount[id] > 0;
	}
};

class Level : public Entity
{
public:
	virtual EntityType Type() { return ET_Level; }
};

class Rocket : public Entity
{
	SpriterSceneSpriter *spriter;
	b2Body *body;
	Entity *owner;
	bool collided;
public:
	Rocket( SceneInfo *info, float x, float y, Entity *_owner ) : owner(_owner), collided(false)
	{
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(x*PixelsToMeters, y*PixelsToMeters);
		bodyDef.fixedRotation = true;
		body = info->world->CreateBody(&bodyDef);
		body->SetUserData(this);

		// Define the dynamic body fixture.
		b2FixtureDef fixtureDef;

		fixtureDef.density = 1.0f;
		fixtureDef.friction = 0.4f;
		fixtureDef.restitution = 0.f;

		b2PolygonShape dynamicBox;
		dynamicBox.SetAsBox(25.f*PixelsToMeters, 10.f*PixelsToMeters, b2Vec2(-25.f*PixelsToMeters,0.f*PixelsToMeters),0.f);
		fixtureDef.shape = &dynamicBox;
		body->CreateFixture(&fixtureDef)->SetUserData( "rocket" );
		body->SetGravityScale( 0.01f );

		spriter = new SpriterSceneSpriter( "rocket\\rocket.scon", "GreyGuyPack\\rocket.json", "\\GreyGuyPack\\rocket.dds" );
		spriter->GetSpacial().sX = 0.1f;
		spriter->GetSpacial().sY = 0.1f;
		info->scene->AddChild( spriter );

		info->toadd.push_back( this );
	}

	virtual void OnDelete( SceneInfo *info )
	{
		info->scene->RemoveChild( spriter );
		info->world->DestroyBody( body );
	}

	virtual void OnContact( Entity *other, float dir[2] )
	{ 
		(void)other; (void)dir; 
		collided = true;
	}

	virtual bool Update( SceneInfo *info, float dt )
	{
		(void)info;
		(void)dt;
		spriter->Update( dt );
		SpriterSpacial &ps = spriter->GetSpacial();
		ps.x = body->GetPosition().x/PixelsToMeters;
		ps.y = body->GetPosition().y/PixelsToMeters;
		ps.sX = body->GetLinearVelocity().x < 0.f ? -0.1f : 0.1f;
		if ( collided )
		{
			camShake = 10.f;
			PlaySound( "Explosion.wav", 1.f, false );
		}
		return !collided;
	}

	virtual bool ShouldCollide( Entity *other )
	{
		return other != owner;
	}

	virtual EntityType Type() { return ET_Rocket; }

	b2Body *GetBody() { return body; }
};

class Enemy : public Entity
{
	b2Body *body;
	SpriterSceneSpriter *spriter;
	bool dead;
public:
	Enemy( SceneInfo *info, float x, float y )
	{
		dead = false;

		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(x*PixelsToMeters, y*PixelsToMeters);
		bodyDef.fixedRotation = true;
		body = info->world->CreateBody(&bodyDef);
		body->SetUserData( this );

		// Define the dynamic body fixture.
		b2FixtureDef fixtureDef;

		fixtureDef.density = 1.0f;
		fixtureDef.friction = 0.4f;
		fixtureDef.restitution = 0.f;

		b2PolygonShape dynamicBox;
		dynamicBox.SetAsBox(25.f*PixelsToMeters, 20.f*PixelsToMeters, b2Vec2(0.f*PixelsToMeters,18.f*PixelsToMeters),0.f);
		fixtureDef.shape = &dynamicBox;
		body->CreateFixture(&fixtureDef)->SetUserData( "enemy" );

		spriter = new SpriterSceneSpriter( "GreyGuy\\enemy.scon", "GreyGuyPack\\allpacks_trim.json", "\\GreyGuyPack\\allpacks_trim.dds" );
		spriter->GetSpacial().sX = 0.3f;
		spriter->GetSpacial().sY = 0.3f;
		info->scene->AddChild( spriter );

		info->toadd.push_back( this );
	}

	virtual bool Update( SceneInfo *info, float dt )
	{
		(void)info;
		spriter->Update( dt );
		SpriterSpacial &ps = spriter->GetSpacial();
		ps.x = body->GetPosition().x/PixelsToMeters;
		ps.y = body->GetPosition().y/PixelsToMeters;
		ps.sX = body->GetLinearVelocity().x > 0.f ? -0.3f : 0.3f;
		if ( body->GetLinearVelocity().x < 0.f )
		{
			body->ApplyLinearImpulse( b2Vec2(-0.03f,0.f), b2Vec2(0.f,0.f), true );
		} else
		{
			body->ApplyLinearImpulse( b2Vec2(0.03f,0.f), b2Vec2(0.f,0.f), true );
		}
		if ( dead && body->IsNoCollision()==false )
		{
			PlaySound( "Hit_hurt4.wav", 1.f, false );
			body->ApplyLinearImpulse( b2Vec2(0.f,2.f), b2Vec2(0.f,0.f), true );
			body->SetNoCollision(true);
		}
		return ps.y > -300.f;
	}

	virtual void OnContact( Entity *other, float dir[2] )
	{ 
		(void)other; (void)dir; 
		if ( other->Type() == ET_Rocket )
			dead = true;
		if ( other->Type() == ET_Player && dir[1] > 0.7f )
			dead = true;
	}

	virtual void OnDelete( SceneInfo *info )
	{
		info->scene->RemoveChild( spriter );
		info->world->DestroyBody( body );
	}
	virtual EntityType Type() { return ET_Enemy; }
};

class Player : public Entity
{
	enum
	{
		PLAYERANIM_JUMPSTART,
		PLAYERANIM_FALLSTART,
		PLAYERANIM_AIR,
		PLAYERANIM_MOVING,
	};

	void PlayerAnimState( bool jumpState, bool crouchState, bool moveState, bool fallState, bool groundState, bool animComplete )
	{
		if ( curanimstate == PLAYERANIM_MOVING )
		{
			if ( moveState )//&& body->GetLinearVelocity().x> 0.f )
			{
				PlaySound( "step.wav", 0.1f, true );
				spriter->SetAnim( "walk" );
			} else
			{
				if ( crouchState )
				{
					spriter->SetAnim( "crouch_idle" );
				} else
				{
					spriter->SetAnim( "idle" );
				}
			}
			if ( jumpState )
			{
				curanimstatecount = 0;
				curanimstate = PLAYERANIM_JUMPSTART;
				PlaySound( "Jump.wav", 1.f, false );
			} else
			if ( fallState )
			{
				curanimstatecount = 0;
				curanimstate = PLAYERANIM_FALLSTART;
			}
		} else
		if ( curanimstate == PLAYERANIM_JUMPSTART )
		{
			curanimstatecount++;
			spriter->SetAnim( "jump_start" );
			if ( animComplete )
			{
				if ( groundState )
				{
					curanimstate = PLAYERANIM_MOVING;
				} else
				if ( body->GetLinearVelocity().y > 0.f )
				{
					curanimstate = PLAYERANIM_AIR;
				} else
				{
					curanimstate = PLAYERANIM_FALLSTART;
				}
			}
		} else
		if ( curanimstate == PLAYERANIM_FALLSTART )
		{
			curanimstatecount++;
			if ( curanimstatecount > 5 )
			{
				spriter->SetAnim( "fall_start" );
			}
			else
			{
				if ( groundState )
					curanimstate = PLAYERANIM_MOVING;
			}
			if ( animComplete )
			{
				if ( groundState )
				{
					PlaySound( "Hit_Hurt4.wav", 0.25f, true );
					curanimstate = PLAYERANIM_MOVING;
				} else
				{
					curanimstate = PLAYERANIM_AIR;
				}
			}
		} else
		if ( curanimstate == PLAYERANIM_AIR )
		{
			if ( body->GetLinearVelocity().y > 0.f )
			{
				spriter->SetAnim( "jump_loop" );
			} else
			{
				spriter->SetAnim( "fall_loop" );
			}
			if ( groundState )
			{
				PlaySound( "Hit_Hurt4.wav", 0.25f, true );
				curanimstate = PLAYERANIM_MOVING;
			}
		}
	}

	int curanimstate;
	int curanimstatecount;
	SpriterSceneSpriter *spriter;
	b2Body *body;
	bool onGround;
	bool wasOnGround;
	int facing;
	float timeToFire;
	int hitPoints;
	bool hitEnemy;
	float enemyHitTime;

public:

	Player( SceneInfo *info, float x, float y ) : onGround(false), wasOnGround(false), facing(1), curanimstate(PLAYERANIM_MOVING), curanimstatecount(0)
	{
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(x*PixelsToMeters, y*PixelsToMeters);
		bodyDef.fixedRotation = true;
		body = info->world->CreateBody(&bodyDef);
		body->SetUserData( this );
		timeToFire = 1.f;

		// Define the dynamic body fixture.
		b2FixtureDef fixtureDef;

		fixtureDef.density = 1.0f;
		fixtureDef.friction = 0.4f;
		fixtureDef.restitution = 0.f;

		b2CircleShape dynamicCircle;
		dynamicCircle.m_p = b2Vec2(0.f,15.f*PixelsToMeters);
		dynamicCircle.m_radius = 15.f*PixelsToMeters;
		fixtureDef.shape = &dynamicCircle;
		body->CreateFixture(&fixtureDef)->SetUserData( "feet" );

		b2PolygonShape dynamicBox;
		dynamicBox.SetAsBox(12.5f*PixelsToMeters, 25.f*PixelsToMeters, b2Vec2(0.f,40.f*PixelsToMeters),0.f);
		fixtureDef.shape = &dynamicBox;
		body->CreateFixture(&fixtureDef)->SetUserData( "body" );

		spriter = new SpriterSceneSpriter( "GreyGuy\\player.scon", "GreyGuyPack\\allpacks_trim.json", "\\GreyGuyPack\\allpacks_trim.dds" );
		spriter->SetAnim( "idle" );
		spriter->SetSkin( "Player" );
		spriter->GetSpacial().sX = 0.3f;
		spriter->GetSpacial().sY = 0.3f;
		info->scene->AddChild( spriter );

		info->toadd.push_back( this );

		hitPoints = 5;
		hitEnemy = false;
		enemyHitTime = 0.f;
	}

	virtual bool Update( SceneInfo *info, float dt )
	{
		(void)info;
		enemyHitTime -= dt;
		if ( hitEnemy && hitPoints > 0 )
		{
			hitPoints--;
			enemyHitTime = 1.f;
			if ( hitPoints == 0 )
			{
				body->SetNoCollision( true );
				body->ApplyLinearImpulse( b2Vec2( 0.f, STB_F(2.f) ), b2Vec2(0.f,0.f), true );
			}
		}

		if ( onGround )
		{
			body->SetLinearDamping( STB_F(1.f) );
		}
		else
		{
			body->SetLinearDamping( STB_F(1.f) );
		}

		bool jump = false;
		bool crouch = false;
		bool move = false;
		if ( onGround && GetAsyncKeyState(VK_SPACE) < 0 )
		{
			jump = true;
 			body->ApplyLinearImpulse( b2Vec2(0.f, STB_F(1.f)), b2Vec2(0.f,0.f), true );
		}
		if ( GetAsyncKeyState(VK_RIGHT) < 0 )
		{
			move = true;
			facing = 1;
			//if ( onGround )
 				body->ApplyLinearImpulse( b2Vec2(STB_F(0.06f), 0.f), b2Vec2(0.f,0.f), true );
		}
		if ( GetAsyncKeyState(VK_LEFT) < 0 )
		{
			move = true;
			facing = -1;
			//if ( onGround )
 				body->ApplyLinearImpulse( b2Vec2(STB_F(-0.06f), 0.f), b2Vec2(0.f,0.f), true );
		}

		if ( GetAsyncKeyState( 'C' ) < 0 )
		{
			crouch = true;
		}
		bool animDone = spriter->Update( dt );
		PlayerAnimState( jump, crouch, move, wasOnGround && !onGround, onGround, animDone );
		wasOnGround = onGround;

		SpriterSpacial &ps = spriter->GetSpacial();
		ps.x = body->GetPosition().x/PixelsToMeters;
		ps.y = body->GetPosition().y/PixelsToMeters;
		ps.sX = facing < 0 ? -0.3f : 0.3f;

		timeToFire -= dt;
		if ( GetAsyncKeyState( 'E' ) < 0 && timeToFire < 0.f )
		{
			timeToFire = 0.25f;

			body->ApplyLinearImpulse( b2Vec2(facing>0.f ? STB_F(-0.08f) : STB_F(0.08f), 0.f), b2Vec2(0.f,0.f), true );
			Rocket *rocket = new Rocket( info, body->GetPosition().x/PixelsToMeters, (body->GetPosition().y/PixelsToMeters)+40.f, this );
			rocket->GetBody()->ApplyLinearImpulse( b2Vec2(facing>0.f ? 1.f : -1.f, 0.f), b2Vec2(0.f,0.f), true );
			PlaySound( "Laser_Shoot6.wav", 1.f, false );
			camShake = 5.f;
		}

		hitEnemy = false;
		onGround = false;
		return true;
	}

	const float *GetPos() const
	{
		return &spriter->GetSpacial().x;
	}

	virtual void OnContact( Entity *other, float normal[2] )
	{
		(void)other;
		onGround |= normal[1] > 0.7f;

		if ( normal[1] < 0.7f && other->Type() == ET_Enemy && enemyHitTime < 0.f )
			hitEnemy |= true;
	}

	void Respawn( float x, float y )
	{
		body->SetNoCollision( false );
		body->SetTransform( b2Vec2(x*PixelsToMeters, y*PixelsToMeters), 0.f );
		body->SetLinearVelocity( b2Vec2(0.f,0.f) );
		spriter->GetSpacial().x = x;
		spriter->GetSpacial().y = y;
		onGround = false;
		wasOnGround = false;
		hitPoints = 5;
	}

	virtual EntityType Type() { return ET_Player; }

	int Facing() { return facing; }
};



class WorldContactsFilter : public b2ContactFilter
{

public:
	virtual bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB)
	{
		if ( fixtureA->GetBody()->IsNoCollision() )
			return false;
		if ( fixtureB->GetBody()->IsNoCollision() )
			return false;

		Entity *enta = (Entity*)fixtureA->GetBody()->GetUserData();
		Entity *entb = (Entity*)fixtureB->GetBody()->GetUserData();
		if ( enta && !enta->ShouldCollide( entb ) )
			return false;
		if ( entb && !entb->ShouldCollide( enta ) )
			return false;
		return true;
	}

};

class GameState
{
public:
	virtual ~GameState() {}
	virtual void Preload()=0;
	virtual void Setup()=0;
	virtual void Update(float dt)=0;
	virtual void Render()=0;
	virtual void Destroy()=0;
};

class GameStateCreator
{
public:
	virtual ~GameStateCreator() {};
	virtual GameState *Create()=0;
};


int amain();

static inline float range(float v, float mn, float mx )
{
	return v < mn ? mn : v > mx ? mx : v;
}

float randf()
{
	return ((rand()/(float)RAND_MAX) - 0.5f) * 2.f;
}

int _main_(int /*_argc*/, char** /*_argv*/)
{
	stb_source_path( "C:\\Dev\\mike\\bgfx-master2\\bgfx\\.build\\win32_vs2010\\bin" );
	//LoadTMX( "level1.tmx" );
	uint32_t width = 1280;
	uint32_t height = 720;
	uint32_t debug = BGFX_DEBUG_TEXT;
	uint32_t reset = BGFX_RESET_VSYNC;

	bgfx::init();
	bgfx::reset(width, height, reset);
	amain();

	// Enable debug text.
	bgfx::setDebug(debug);

	// Set view 0 clear state.
	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR_BIT|BGFX_CLEAR_DEPTH_BIT
		, 0x303030ff
		, 1.0f
		, 0
		);

	// Setup root path for binary shaders. Shader binaries are different 
	// for each renderer.
	switch (bgfx::getRendererType() )
	{
	default:
	case bgfx::RendererType::Direct3D9:
		s_shaderPath = "shaders/dx9/";
		break;

	case bgfx::RendererType::Direct3D11:
		s_shaderPath = "shaders/dx11/";
		break;

	case bgfx::RendererType::OpenGL:
		s_shaderPath = "shaders/glsl/";
		s_flipV = true;
		break;

	case bgfx::RendererType::OpenGLES2:
	case bgfx::RendererType::OpenGLES3:
		s_shaderPath = "shaders/gles/";
		s_flipV = true;
		break;
	}

	// Create vertex stream declaration.
	s_PosColorTexCoord0Decl.begin();
	s_PosColorTexCoord0Decl.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
	s_PosColorTexCoord0Decl.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true);
	s_PosColorTexCoord0Decl.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
	s_PosColorTexCoord0Decl.end();  

	//bgfx::UniformHandle u_time = bgfx::createUniform("u_time", bgfx::UniformType::Uniform1f);

	// Load diffuse texture.

	bgfx::UniformHandle u_texColor = bgfx::createUniform("u_texColor", bgfx::UniformType::Uniform1iv);

	bgfx::ProgramHandle raymarching = loadProgram("vs_helloworld", "fs_helloworld");

	Box2DDebugDraw b2DebugDraw( raymarching );
	SpriterSceneNode scene;

	SpriterSceneNode bg;
	scene.AddChild( &bg );

	//SpriterSceneSVG level("drawing.svg");
	//scene.AddChild( &level );

	SpriterSceneNode bg2;
	scene.AddChild( &bg2 );
	//CreateScene(&bg2,"drawing.svg","|Background");

	std::vector<float> baseLoc;
	for (int i=0; i<bg2.GetNumKids(); i++)
	{
		baseLoc.push_back( bg2.GetKid(i)->GetSpacial().x );
		baseLoc.push_back( bg2.GetKid(i)->GetSpacial().y );
	}

	SpriterSceneNode fg2;
	scene.AddChild( &fg2 );
	//CreateScene(&fg2,"drawing.svg","|Foreground");


	SpriterSceneNode fg3;
	scene.AddChild( &fg3 );
	CreateTiledScene( &fg3, "mylevel1.tmx", "bg0" );
	CreateTiledScene( &fg3, "mylevel1.tmx", "bg1" );
	CreateTiledScene( &fg3, "mylevel1.tmx", "fg" );

	WorldContacts contacts;
	WorldContactsFilter filter;
	// Define the gravity vector.
	b2Vec2 gravity(0.0f, -20.0f);
	// Construct a world object, which will hold and simulate the rigid bodies.
	b2World world(gravity);
	world.SetDebugDraw( &b2DebugDraw );
	world.SetContactListener( &contacts );
	world.SetContactFilter( &filter );
	
	Level level;
	CreateTiledBox2dCollider( world, &level, "mylevel1.tmx", "collision" );
	// /*b2Body* groundBody = */CreateBox2DCollider( world, &level, "drawing.svg","|Collision");//.CreateBody(&groundBodyDef);
	float playerSpawn[2];
	float enemySpawn[2][2];
	FindTiledEntity( playerSpawn, "mylevel1.tmx", "objlayer", "playerborn" );
//	FindEntity( playerSpawn, "drawing.svg", "|Entities", "playerstart" );
	FindEntity( enemySpawn[0], "drawing.svg", "|Entities", "spawner0" );
	FindEntity( enemySpawn[1], "drawing.svg", "|Entities", "spawner1" );

	SpriterSceneSpriteSheet sheet("mylevel1_tiles.png",32,32);
	sheet.AddAnim( "{\"name\":\"test\",\"loop\":0,\"rateMS\":33,\"frames\":[0,1,2,3]}" );
	sheet.GetSpacial().x = playerSpawn[0];
	sheet.GetSpacial().y = playerSpawn[1];
	scene.AddChild( &sheet );

	float enemySpawnTime = 10.f;

	SceneInfo sceneInfo;
	sceneInfo.scene = &scene;
	sceneInfo.world = &world;
	sceneInfo.contacts = &contacts;

	Player *player = new Player( &sceneInfo, playerSpawn[0], playerSpawn[1] );

	sceneInfo.entities.insert( sceneInfo.entities.end(), sceneInfo.toadd.begin(), sceneInfo.toadd.end() );
	sceneInfo.toadd.clear();

	stb_mixhigh_init(20000,0.005, 88200*4);
	stb_mixlow_global_volume(1.f);

	float camOx = playerSpawn[0];//range(playerPos[0],1280.f/2.f,1980.f-1280.f/2.f);
	float camOy = playerSpawn[1];//range(playerPos[1],720.f/2.f,1080.f-720.f/2.f);

	Music music;
	LoadMusic( music, "4matMix2.ogg" );
	PlayMusic( music );

	int64_t frameTimeErr = 0;
	int64_t currentTime = 0;
	while (!entry::processEvents(width, height, debug, reset) )
	{
		UpdateMusic( music );
		stb_mixhigh_step(5000);

		// Set view 0 default viewport.
		bgfx::setViewRect(0, 0, 0, width, height);

		// Set view 1 default viewport.
		bgfx::setViewRect(1, 0, 0, width, height);

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to viewZ 0.
		bgfx::submit(0);

		int64_t now = bx::getHPCounter();
		static int64_t last = now;
		const int64_t frameTime = ((now - last)*1000)/bx::getHPFrequency();
		frameTimeErr += frameTime;
		last = now;
		const double freq = double(bx::getHPFrequency() );
		const double toMs = 1000.0/freq;
		
		const int64_t delta = 1000/60;
		while ( frameTimeErr >= delta )
		{
			UpdateMusic( music );
			stb_mixhigh_step(5000);
			enemySpawnTime -= 1.f/60.f;
			if ( enemySpawnTime < 0.f )
			{
				float *pos = enemySpawn[rand()%2];
				enemySpawnTime = 2.f;
				new Enemy( &sceneInfo, pos[0], pos[1] );
			}
			for (std::vector<Entity*>::iterator c=sceneInfo.entities.begin(); c!=sceneInfo.entities.end(); )
			{
				if ( (*c)->Update( &sceneInfo, 1.f/60.f ) )
				{
					c++;
				}
				else
				{
					(*c)->OnDelete( &sceneInfo );
					delete (*c);
					c = sceneInfo.entities.erase(c);
				}
			}
			sceneInfo.entities.insert( sceneInfo.entities.end(), sceneInfo.toadd.begin(), sceneInfo.toadd.end() );
			sceneInfo.toadd.clear();

			float const *playerPos = player->GetPos();
			if ( playerPos[1] < (-300.f) )
			{
				camOx = playerSpawn[0];//range(playerPos[0],1280.f/2.f,1980.f-1280.f/2.f);
				camOy = playerSpawn[1];//range(playerPos[1],720.f/2.f,1080.f-720.f/2.f);
				player->Respawn( playerSpawn[0], playerSpawn[1] );
			}

			world.Step( 1.f/60.f, 4, 4 );
			{
				for ( const b2Body *body = world.GetBodyList(); body; body = body->GetNext() )
				{
					for ( const b2ContactEdge *contactEdge = body->GetContactList(); contactEdge; contactEdge = contactEdge->next )
					{
						const b2Contact *contact = contactEdge->contact;

						if ( !contact->IsTouching() )
							continue;
						if ( !contact->IsEnabled() )
							continue;

						b2WorldManifold manifold;
						contact->GetWorldManifold( &manifold );
						Entity *enta = (Entity*)contact->GetFixtureA()->GetBody()->GetUserData();
						Entity *entb = (Entity*)contact->GetFixtureB()->GetBody()->GetUserData();
						if ( enta )
							enta->OnContact( entb, &manifold.normal.x );
						if ( entb )
							entb->OnContact( enta, &manifold.normal.x );
					}
				}
			}

			currentTime += delta;
			frameTimeErr -= delta;
			camShake = range( camShake-camShakeDecay*(1.f/60.f), 0.f, 10.f );
		}

		stb_mixhigh_step(5000);

		// Use debug font to print information about this example.
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 1, 0x0f, "Frame: % 7.3f[ms]", double(frameTime)*toMs);

		float at[3] = { 0.0f, 0.0f, 0.0f };
		float eye[3] = { 0.0f, 0.0f, -15.0f };
		
		float view[16];
		float proj[16];
		mtxLookAt(view, eye, at);
		mtxProj(proj, 60.0f, float(width)/float(height), 0.1f, 100.0f);

		// Set view and projection matrix for view 0.
		bgfx::setViewTransform(0, view, proj);

		float const *playerPos = player->GetPos();
		float ortho[16];
		float camTargetOx = playerPos[0] + player->Facing() * 200.f;//range(playerPos[0],1280.f/2.f,1980.f-1280.f/2.f);
		float camTargetOy = playerPos[1];//range(playerPos[1],720.f/2.f,1080.f-720.f/2.f);
		float factor = 0.9f;
		camOx = factor * camOx + (1.f-factor)*camTargetOx;//range(playerPos[0],1280.f/2.f,1980.f-1280.f/2.f);
		camOy = factor * camOy + (1.f-factor)*camTargetOy;//range(playerPos[1],720.f/2.f,1080.f-720.f/2.f);
		camOx = range( camOx, playerPos[0]-400.f, playerPos[0]+400.f );
		camOy = range( camOy, playerPos[1]-400.f, playerPos[1]+400.f );
		camOx += randf() * camShake;
		camOy += randf() * camShake;

		mtxOrtho(ortho, camOx+0.0f-1280.f/2.f, camOx+1280.0f-1280.f/2.f, camOy+0.f-720.f/2.f, camOy+720.0f-720.f/2.f, 0.0f, 100.0f);

		for (int i=0; i<bg2.GetNumKids(); i++)
		{
			float parallaxScale = (i+1.f)/bg2.GetNumKids()*0.1f;
			bg2.GetKid(i)->GetSpacial().x = baseLoc[i*2+0] - camOx * parallaxScale;
			bg2.GetKid(i)->GetSpacial().y = baseLoc[i*2+1] - camOy * parallaxScale;
		}

		// Set view and projection matrix for view 1.
		bgfx::setViewTransform(1, NULL, ortho);

		SpriterSceneState state;
		state.raymarching = raymarching;
		state.u_texColor = u_texColor;
		state.spacial.angle = 0.f;
		state.spacial.sX = 1.f;
		state.spacial.sY = 1.f;
		state.spacial.x = 0.f;
		state.spacial.y = 0.f;
		scene.Render( state );
		world.DrawDebugData();

		// Advance to next frame. Rendering thread will be kicked to 
		// process submitted rendering primitives.
		bgfx::frame();
		stb_mixhigh_step(5000);
	}

	// Cleanup.
	bgfx::destroyProgram(raymarching);
	bgfx::destroyUniform(u_texColor);

	// Shutdown bgfx.
	bgfx::shutdown();

	return 0;
}


int luaCallNameSpaceFunc( lua_State *L, const char *ns, const char *f )
{
  int t0 = lua_gettop(L);
  lua_pushstring(L,ns); // Game
  lua_gettable(L, LUA_GLOBALSINDEX); // GameTable
  lua_pushstring(L,f); // GameTable preload
  lua_gettable(L, -2); // GameTable preloadfunc
  int res=lua_pcall(L, 0, 0, 0); // GameTable
  int t5 = lua_gettop(L);
  lua_pop(L,t5-t0); //
  return res;
}

template<class T>
struct LuaType
{
	//static void Push( lua_State *L, const T & t );
};

template<>
struct LuaType<float>
{
	static void Push( lua_State *L, const float t )
	{
		lua_pushnumber(L,t);
	}
};

template <class P0>
int luaCallNameSpaceFunc( lua_State *L, const char *ns, const char *f, const P0 p0 )
{
  int t0 = lua_gettop(L);
  lua_pushstring(L,ns); // Game
  lua_gettable(L, LUA_GLOBALSINDEX); // GameTable
  lua_pushstring(L,f); // GameTable preload
  lua_gettable(L, -2); // GameTable preloadfunc
  LuaType<P0>::Push(L,p0);
  int res=lua_pcall(L, 1, 0, 0); // GameTable
  int t5 = lua_gettop(L);
  lua_pop(L,t5-t0); //
  return res;
}

LUABINDY(SpriterSceneNode,Root)
static int LuaThunkAddChild( LuaSpriterSceneNode *obj, lua_State *L )
{
	obj->AddChild( Lunar<LuaSpriterSceneNode>::check( L, 1 ) );
	return 0;
}
LuaSpriterSceneNode::LuaSpriterSceneNode(lua_State *)
{ 
}
LUABINDYBEGIN(SpriterSceneNode)
  LUNAR_DECLARE_METHOD(AddChild),
LUABINDYEND()

LUABINDY(SpriterSceneSprite,SpriterSceneNode)
LuaSpriterSceneSprite::LuaSpriterSceneSprite(lua_State *L) : SpriterSceneSprite( luaL_checkstring(L,1) )
{ 
}
LUABINDYBEGIN(SpriterSceneSprite)
LUABINDYEND()

LUABINDY(SpriterSceneSpriteSheet,SpriterSceneNode)
LuaSpriterSceneSpriteSheet::LuaSpriterSceneSpriteSheet(lua_State *L) : SpriterSceneSpriteSheet( luaL_checkstring(L,1), luaL_checkint(L,2), luaL_checkint(L,3) )
{ 
}
LUABINDYBEGIN(SpriterSceneSpriteSheet)
LUABINDYEND()

LUABINDY(SpriterSceneSVG,SpriterSceneNode)
LuaSpriterSceneSVG::LuaSpriterSceneSVG(lua_State *L) : SpriterSceneSVG( luaL_checkstring(L,1) )
{ 
}
LUABINDYBEGIN(SpriterSceneSVG)
LUABINDYEND()

LUABINDY(SpriterSceneSpriter,SpriterSceneNode)
LuaSpriterSceneSpriter::LuaSpriterSceneSpriter(lua_State *L) : SpriterSceneSpriter( luaL_checkstring(L,1), luaL_checkstring(L,2), luaL_checkstring(L,3) )
{ 
}
static int LuaThunkSetAnim( LuaSpriterSceneSpriter *obj, lua_State *L )
{
	obj->SetAnim( luaL_checkstring(L,1) );
	return 0;
}
static int LuaThunkSetSkin( LuaSpriterSceneSpriter *obj, lua_State *L )
{
	obj->SetSkin( luaL_checkstring(L,1) );
	return 0;
}
static int LuaThunkSetTime( LuaSpriterSceneSpriter *obj, lua_State *L )
{
	obj->SetTime( (float)luaL_checknumber(L,1) );
	return 0;
}
static int LuaThunkUpdate( LuaSpriterSceneSpriter *obj, lua_State *L )
{
	obj->Update( (float)luaL_checknumber(L,1) );
	return 0;
}
LUABINDYBEGIN(SpriterSceneSpriter)
  LUNAR_DECLARE_METHOD(SetAnim),
  LUNAR_DECLARE_METHOD(SetSkin),
  LUNAR_DECLARE_METHOD(SetTime),
  LUNAR_DECLARE_METHOD(Update),
LUABINDYEND()



LUALIB_API int luagfx_render(lua_State *L) 
{
	float camX = luaL_checknumber( L, 1 );
	float camY = luaL_checknumber( L, 2 );
	LuaSpriterSceneNode *node = Lunar<LuaSpriterSceneNode>::check(L, 3);
	(void)camX;
	(void)camY;
	(void)node;
	return 0;
}


static const luaL_Reg gfxlib[] = {
  {"render",   luagfx_render},
  {NULL, NULL}
};


LUALIB_API int luaopen_gfx(lua_State *L) {
  luaL_register(L, "Gfx", gfxlib);
  return 1;
}


static const luaL_Reg mylualibs[] = {
  {"Gfx", luaopen_gfx},
  {NULL, NULL}
};


LUALIB_API void luaL_openmylibs (lua_State *L, const luaL_Reg *libs) {
  const luaL_Reg *lib = libs;
  for (; lib->func; lib++) {
    lua_pushcfunction(L, lib->func);
    lua_pushstring(L, lib->name);
    lua_call(L, 1, 0);
  }
}


int amain()
{
  lua_State *L = luaL_newstate();

  luaL_openlibs(L);
  luaL_openmylibs(L, mylualibs);

  Lunar<LuaSpriterSceneNode>::Register(L);
  Lunar<LuaSpriterSceneSprite>::Register(L);

  int res = luaL_dofile(L, "test.lua");
  res = luaCallNameSpaceFunc(L, "Game", "preload" );

  res = luaCallNameSpaceFunc(L, "Game", "update", 1.f/60.f );

  res = luaCallNameSpaceFunc(L, "Game", "render" );
  (void)res;


  lua_gc(L, LUA_GCCOLLECT, 0);  // collected garbage
  lua_close(L);
  return 0;
}