extends Node
# local peer
var peer = ENetMultiplayerPeer.new()
# local username
var _username: String 

@onready var world: Node2D = $"../GameWorld"
@export var character_creation_scene: PackedScene

var _lobby_interface: LobbyInterface
var _players: Array = []
var _peers: Array = []
var _ready_peers: Array = []
var _characters_spawn_data: Array = [] #hacer array de dict



func _on_menu_control_lobby_started(lobby_interface: LobbyInterface, joined_ip: String) -> void:
	_lobby_interface = lobby_interface
	_lobby_interface.update_username(_username)
	_lobby_interface.ready_toggled.connect(_on_player_ready)
	_lobby_interface.race_selected.connect(_on_race_selected)
	_lobby_interface.sex_selected.connect(_on_sex_selected)
	_lobby_interface.head_selected.connect(_on_head_selected)
	_lobby_interface.class_selected.connect(_on_class_selected)
	_lobby_interface.follower_selected.connect(_on_follower_selected)
	#_lobby_interface.follower_body_selected.connect(_on_follower_body_selected)
	
	if not joined_ip:
		_lobby_interface.set_up_host_lobby(_username)
		_lobby_interface.player_clicked_leave.connect(_cancel_host, CONNECT_ONE_SHOT)
		_host()
	else:
		_lobby_interface.set_up_joiner_lobby()
		_lobby_interface.player_clicked_leave.connect(_leave_as_client, CONNECT_ONE_SHOT)
		_join(joined_ip)

func _host() -> void:
	peer.create_server(135)
	multiplayer.multiplayer_peer = peer
	multiplayer.peer_connected.connect(_on_player_join)
	multiplayer.peer_disconnected.connect(_on_player_disconnect)
	_players.push_back(_username)
	_peers.push_back(1)
	_characters_spawn_data.push_back({})
	

func _join(ip: String) -> void:
	peer.create_client(ip, 135)
	multiplayer.multiplayer_peer = peer
	#TODO: check if connection is successful
	
	
# executed ONLY server-side 
func _on_player_join(peer_id: int) -> void:
	await get_tree().create_timer(0.5).timeout
	_lobby_interface._update_lobby_title_for_client(peer_id)
	_request_player_username(peer_id)
	_players.push_back(await player_username_received)
	_peers.push_back(peer_id)
	_characters_spawn_data.push_back({})
	_sync_state_for_clients()
	_update_players_for_gui()
	
func _on_player_disconnect(peer_id: int) -> void:
	var player_i: int = _peers.find(peer_id)
	_players.remove_at(player_i)
	_characters_spawn_data.remove_at(player_i)
	_peers.erase(peer_id)
	_ready_peers.erase(peer_id)
	_sync_state_for_clients()
	_update_players_for_gui()
# executed ONLY server-side 
	
func _sync_state_for_clients() -> void: 
	_give_player_list.rpc(_players) 
	_give_peer_list.rpc(_peers)
	_give_ready_peers_list.rpc(_ready_peers)
	_give_characters_spawn_data.rpc(_characters_spawn_data)
	
@rpc func _give_player_list(players: Array) -> void: _players = players; _update_players_for_gui()
@rpc func _give_peer_list(peers: Array) -> void: _peers = peers
@rpc func _give_ready_peers_list(ready_peers: Array) -> void: _ready_peers = ready_peers	
@rpc func _give_characters_spawn_data(characters_spawn_data: Array) -> void: _characters_spawn_data = characters_spawn_data
	
func _cancel_host() -> void:
	clear_arrays()
	_remove_from_lobby.rpc(false)
	await get_tree().create_timer(1).timeout
	peer.close()
	peer = ENetMultiplayerPeer.new()
		
func _leave_as_client() -> void:
	peer.close()
	clear_arrays()
		
func clear_arrays() -> void:
	_players.clear()
	_peers.clear()
	_ready_peers.clear()
	_characters_spawn_data.clear()

signal removed_from_lobby(kicked: bool)
@rpc
func _remove_from_lobby(kicked: bool) -> void:
	clear_arrays()
	removed_from_lobby.emit(kicked)

func _update_players_for_gui() -> void:
	if is_instance_valid(_lobby_interface):
		_lobby_interface.update_player_list(_players)
	
# when ready is pressed in the GUI
func _on_player_ready(ready: bool) -> void:
	if multiplayer.get_unique_id() != 1:
		_peer_is_ready.rpc(ready)
		
	elif _is_everybody_ready():
		print("starting")
		print(_characters_spawn_data)

@rpc("call_local", "any_peer")
func _peer_is_ready(ready: bool) -> void:
	var peer_id: int = multiplayer.get_remote_sender_id()
	if ready and peer_id not in _ready_peers:
		_ready_peers.push_back(peer_id)
	elif not ready:
		_ready_peers.erase(peer_id)
	
func _is_everybody_ready() -> bool:
	return _ready_peers.size() + 1 == _peers.size()
		
	
# EN VEZ DE TODO ESTO HACER Q APENAS SE UNA EL PLAYER ESTE EJECUTE UN .RPC_ID(1, _username) Y EL SERVER SE LO QUEDA AHÍ
# for requesting a peer's username
signal player_username_received(username: String)
var requested_peer: int = -1
func _request_player_username(peer_id: int) -> void:
	requested_peer = peer_id
	_return_player_username.rpc_id(peer_id)
@rpc 
func _return_player_username() -> void:
	_receive_player_username.rpc_id(multiplayer.get_remote_sender_id(), _username)
@rpc("any_peer")
func _receive_player_username(username: String) -> void:
	if  multiplayer.get_remote_sender_id() == requested_peer:
		player_username_received.emit(username)
	requested_peer = -1
# for requesting a peer's username


# ----------------------------------------------------------------------------------
# ----------------------- character creation synchronization -----------------------
# ----------------------------------------------------------------------------------
func _on_race_selected(race: ControllableRace):
	if race: _update_characterization_for_everyone.rpc("race", race.id)
	else: _update_characterization_for_everyone.rpc("race")
	
func _on_sex_selected(sex: Enums.Sex):
	if sex > 0: _update_characterization_for_everyone.rpc("sex", sex)
	else: _update_characterization_for_everyone.rpc("sex")
	
func _on_head_selected(i: int):
	if i >= 0: _update_characterization_for_everyone.rpc("head", i)
	else: _update_characterization_for_everyone.rpc("head")
	
func _on_class_selected(klass: Class):
	if klass: _update_characterization_for_everyone.rpc("klass", klass.id)
	else: _update_characterization_for_everyone.rpc("klass")
	
func _on_follower_selected(follower: UncontrollableRace):
	if follower: _update_characterization_for_everyone.rpc("follower", [follower.id])
	else: _update_characterization_for_everyone.rpc("follower")
	
@rpc("call_local", "any_peer")
func _update_characterization_for_everyone(characterization_key: String, value = null): 
	var sender_i: int = _peers.find(multiplayer.get_remote_sender_id())
	if value:
		_characters_spawn_data[sender_i][characterization_key] = value
	else:
		_characters_spawn_data[sender_i].erase(characterization_key)

# used by main.gd
func update_username(username: String):
	_username = username
