
Explosion = Explosion or {}

function Explosion:new( gfxnode, x, y )
	local obj = {}
	setmetatable( obj, self ); 
	self.__index = self;

	obj.sprite = SpriterSceneSprite:new( "boom3_0.png" )
	obj.sprite:SetFrame( 0, 128, 128 )
	gfxnode:AddChild( obj.sprite );
	obj.sprite:SetPos( x, y );
	obj.x = x
	obj.y = y
	obj.time = 0
	Spriter.PlaySound( "Explosion.wav", 0.5, 0 );

	return obj
end

function Explosion:updateFixed( g, dt)
	self.time = self.time + dt;
	local frame = self.time * 60
	self.sprite:SetFrame( frame, 128, 128 )
	if frame >= 64 then
		return 1
	end

	return 0
end

function Explosion:remove( )
	self.sprite:Remove()
end

Player = Player or {}

function Player:new( gfxnode )
	local obj = {}
	setmetatable( obj, self ); 
	self.__index = self;

	obj.sprite = SpriterSceneSprite:new( "space.png" )
	obj.sprite:SetPixelRect( 90, 6, 141, 72 )
	gfxnode:AddChild( obj.sprite );
	obj.ax = 0
	obj.ay = 0
	obj.vx = 0
	obj.vy = 0
	obj.x = 0
	obj.y = 0
	obj.body = b2DynamicBody:new( Game.physicsWorld )
	obj.body:AddBoxCollision( "ship", 50, 50, 0, 0, 0 )

	return obj
end

function Player:updateFixed( g, dt)

	self.vx = self.vx * 0.9
	self.vy = self.vy * 0.9
	self.ax = 0
	self.ay = 0

	local scaleX = 1
	local scaleY = 1
	if Spriter.ControllerPressed( "left" )~=0 then
		scaleX = 0.9
		self.ax = -3000
	end
	if Spriter.ControllerPressed( "right" )~=0 then
		scaleX = 0.9
		self.ax = 3000
	end
	if Spriter.ControllerPressed( "up" )~=0 then
		scaleY = 0.9
		self.ay = 3000
	end
	if Spriter.ControllerPressed( "down" )~=0 then
		scaleY = 0.9
		self.ay = -3000
	end
	self.sprite:SetScale( scaleX, scaleY )

	self.vx = self.vx + self.ax * dt
	self.vy = self.vy + self.ay * dt
	self.x = self.x + self.vx * dt
	self.y = self.y + self.vy * dt

	self.x, self.y = self.body:GetPos()

	self.sprite:SetPos( self.x, self.y );

	return 0
end

function Player:explode( g )
	g.AddEffect( Explosion:new( g.layerEffects, self.x, self.y ) )
end

Bullet = Bullet or {}

function Bullet:new( gfxnode, x, y, a )
	local obj = {}
	setmetatable( obj, self ); 
	self.__index = self;

	obj.time = 1
	obj.x = x;
	obj.y = y;
	obj.angle = a
	obj.sprite = SpriterSceneSprite:new( "space.png" )
	obj.sprite:SetPixelRect( 164, 11, 179, 44 )
	gfxnode:AddChild( obj.sprite );
	Spriter.PlaySound( "sfx.wav", 0.1, 0 );

	return obj
end

function Bullet:updateFixed( g, dt)
	self.time = self.time - dt
	self.x = self.x - math.sin( self.angle ) * 5
	self.y = self.y + math.cos( self.angle ) * 5
	self.sprite:SetPos( self.x, self.y );
	self.sprite:SetAngle( self.angle )

	if ( self.time < 0 ) then
		return 1
	end
	return 0
end

function Bullet:update( g, dt)
	return 0
end

function Bullet:remove( )
	self.sprite:Remove()
end

Orb = Orb or {}

function Orb:new( gfxnode, x, y )
	local obj = {}
	setmetatable( obj, self ); 
	self.__index = self;

	obj.firetime = 1
	obj.x = x;
	obj.y = y;
	obj.angle = 0
	obj.sprite = SpriterSceneSprite:new( "space.png" )
	obj.sprite:SetPixelRect( 7, 1, 73, 67 )
	gfxnode:AddChild( obj.sprite );

	return obj
end

function Orb:updateFixed( g, dt)
	self.firetime = self.firetime - dt
	if self.firetime < 0 then
		self.firetime = 0.15
		g.AddEntity( Bullet:new( g.layer1, self.x, self.y, self.angle ) )
	end

	self.angle = self.angle + 1.5 * dt
	self.sprite:SetPos( self.x, self.y );
	self.sprite:SetAngle( self.angle )

	return 0
end

function Orb:update( g, dt)
	return 0
end

function Orb:remove()
	self.sprite:Remove()
end



Game = Game or {}

function Game.AddEntity( e )
	Game.entities[#Game.entities+1] = e
end

function Game.AddEffect( e )
	Game.effects[#Game.effects+1] = e
end

function Game.preload()
	Game.entities = {}
	Game.effects = {}

	Game.scene = SpriterSceneNode:new()
	Game.layer0 = SpriterSceneNode:new()
	Game.layer1 = SpriterSceneNode:new()
	Game.layer2 = SpriterSceneNode:new()
	Game.layerEffects = SpriterSceneNode:new()
	Game.gui = SpriterSceneNode:new()
	Game.physicsWorld = b2World:new( 0, -10 )
	Game.physicsWorldBody = b2StaticBody:new( Game.physicsWorld )
	Game.physicsWorldBody:AddBoxCollision( "land", 720, 10, 0, -200, 0 )

	Game.scene:AddChild( Game.layer0 );
	Game.scene:AddChild( Game.layer1 );
	Game.scene:AddChild( Game.layer2 );
	Game.scene:AddChild( Game.layerEffects );
	Game.scene:AddChild( Game.gui );

	Game.spacebg = SpriterSceneSprite:new( "spacebg.png" )
	Game.layer0:AddChild( Game.spacebg )

	Game.player = Player:new( Game.layer2 );
	Game.AddEntity( Orb:new( Game.layer2, 200,200 ) )
	Game.AddEntity( Orb:new( Game.layer2, 100,-200 ) )
	Game.AddEntity( Orb:new( Game.layer2, -200,10 ) )

	Game.health = SpriterSceneText:new( "font/visitor1.ttf", 32 );
	Game.health:SetText( "testing" );
	Game.gui:AddChild( Game.health );

	Spriter.LoadMusic( "4matMix2.ogg" )
	Spriter.PlayMusic()
end

function Game.updateFixed( dt )
	Game.physicsWorld:Step( dt )
	Game.player:updateFixed(Game,dt)
	local i=1
	while i<=#Game.effects do
		if Game.effects[i]:updateFixed(Game,dt)~=0 then
			Game.effects[i]:remove()
			Game.effects[i]=Game.effects[#Game.effects]
			table.remove(Game.effects, #Game.effects)
		else
			i = i + 1
		end
	end

	i=1
	while i<=#Game.entities do
		if Game.entities[i]:updateFixed(Game,dt)~=0 then
			Game.entities[i]:remove()
			Game.entities[i]=Game.entities[#Game.entities]
			table.remove(Game.entities, #Game.entities)
		else

			local dx = Game.player.x - Game.entities[i].x
			local dy = Game.player.y - Game.entities[i].y
			local distSq = dx*dx + dy*dy
			if distSq < 500 then
				Game.player:explode( Game )
				Game.entities[i]:remove()
				Game.entities[i]=Game.entities[#Game.entities]
				table.remove(Game.entities, #Game.entities)
			else
				i=i+1
			end
		end
	end
end

function Game.update( dt )
	Game.player:update(Game,dt)
	local i=1
	while i<=#Game.entities do
		if Game.entities[i]:update(Game,dt)~=0 then
			Game.entities[i]:remove()
			Game.entities[i]=Game.entities[#Game.entities]
			table.remove(Game.entities, #Game.entities)
		else
			i=i+1
		end
	end
end

function Game.render()
	Spriter.render( 0, 0, Game.scene )
end
