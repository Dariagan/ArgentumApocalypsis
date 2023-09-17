extends Node
# PONER EN LA GUI ARRIBA A LA DERECHA DAYS SURVIVED: X, EN FUENTE DIABLESCA

@onready var tile_map: ProceduralTilemap = $TileMap

@export var being_scene: PackedScene
@onready var multiplayer_spawner: MultiplayerSpawner = $MultiplayerSpawner


func start_new_game(players_start_data: Array, peers: Array) -> void:

	tile_map.generate_world.rpc()

	var i: int = 0
	for player_start_data in players_start_data:
		player_start_data = player_start_data as Dictionary
		
		if not player_start_data.has("race"):
			player_start_data["race"] = "controllable_random"
		if not player_start_data.has("sex"):
			player_start_data["sex"] = "random"
		if not player_start_data.has("klass"):
			player_start_data["klass"] = "random"
		if not player_start_data.has("head"):
			player_start_data["head"] = "random"
		if not player_start_data.has("body"):
			player_start_data["body"] = "random"
		
		player_start_data["faction"] = "player"	
		
		var being_spawn_data = BeingSpawnData.new(player_start_data)
		
		var being: Being = being_scene.instantiate()
		being.name = str(peers[i])
		being.position.x = i*40
		tile_map.spawn_active_being(being)
		being.construct(being_spawn_data)
		
		being.give_control.rpc(peers[i])
		
		i+=1
		
	
