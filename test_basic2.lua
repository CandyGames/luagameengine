
Rocket = Rocket or {}

function Rocket:new()
	local obj = {}
	setmetatable( obj, self ); 
	self.__index = self;
	return obj
end

function Rocket:updateFixed(g,dt)
	return 0
end

function Rocket:update(g,dt)
	print("rocket")
	return 0
end

Player = Player or {}

function Player:new( gfxnode )
	local obj = {}
	setmetatable( obj, self ); 
	self.__index = self;

	obj.sprite = SpriterSceneSpriter:new( "GreyGuy\\player.scon", "GreyGuyPack\\allpacks_trim.json", "\\GreyGuyPack\\allpacks_trim.dds" )
	gfxnode:AddChild( obj.sprite );

	return obj
end

function Player:updateFixed( g, dt)
	self.sprite:Update( dt )

	if Spriter.ControllerPressed( "left" )~=0 then
		self.sprite:SetScale( -1, 1 )
	end

	if Spriter.ControllerPressed( "right" )~=0 then
		self.sprite:SetScale( 1, 1 )
	end

	if Spriter.ControllerPressed( "fire" )~=0 then
		self.sprite:SetAnim( "walk" )
	end

	return 0
end

function Player:update( g, dt)
	print(dt)
	return 0
end

function Player:remove()
	print("remove Player")
end



Game = Game or {}

function Game.AddEntity( e )
	Game.entities[#Game.entities+1] = e
end

function Game.preload()
	Game.entities = {}

	Game.scene = SpriterSceneNode:new()
	Game.layer0 = SpriterSceneNode:new()
	Game.layer1 = SpriterSceneNode:new()
	Game.layer2 = SpriterSceneNode:new()

	Game.scene:AddChild( Game.layer0 );
	Game.scene:AddChild( Game.layer1 );
	Game.scene:AddChild( Game.layer2 );

	Game.AddEntity( Player:new( Game.layer2 ) )
end

function Game.updateFixed( dt )
	local i=1
	while i<=#Game.entities do
		if Game.entities[i]:updateFixed(Game,dt)~=0 then
			Game.entities[i]:remove()
			Game.entities[i]=Game.entities[#Game.entities]
			table.remove(Game.entities, #Game.entities)
		else
			i=i+1
		end
	end
end

function Game.update( dt )
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
	Spriter.render( 1, 2, Game.scene )
end
