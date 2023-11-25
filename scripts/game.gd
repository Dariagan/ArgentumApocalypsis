@tool
extends Node
# PONER EN LA GUI ARRIBA A LA DERECHA DAYS SURVIVED: X, EN FUENTE DIABLESCA

@onready var tile_map: GdTileMap = $ArgentumTileMap

@export var being_scene: PackedScene:
	set(value): being_scene = value; update_configuration_warnings()

func _ready() -> void:
	if GlobalData.insta_start:
		start_new_game([{}], [1])

func start_new_game(players_start_data: Array, peers: Array) -> void:
	generate_world.rpc()
	
	var i: int = 0
	for player_start_data: Dictionary in players_start_data:
		
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
		
		player_start_data["fac"] = "player"	
		
		var player_being_init_data = BeingReqInitData.new()
		player_being_init_data.construct(player_start_data)
		
		var being: Being = being_scene.instantiate()
		being.name = str(peers[i])
		tile_map.spawn_starting_player(being)
		
		being.construct(player_being_init_data)
		await get_tree().create_timer(0.0001).timeout
		being.give_control.rpc(peers[i])
		
		i+=1
		
@rpc("call_local")
func generate_world() -> void:
	tile_map.generate_world()

func _get_configuration_warnings():
	if being_scene == null: return ["being_scene must not be empty!"]
	else: return []

