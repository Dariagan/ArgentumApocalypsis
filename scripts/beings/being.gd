extends CharacterBody2D

#todo separar funciones en componentes hijos?
class_name Being
#para persistirlo habrá que usar packedscene para guardar las cosas custom children q puede ser q tenga, sino habrá q iterar por cada child

var uid: int = randi_range(-9223372036854775808, 9223372036854775807)

@export var acceleration = 30

@export var friction = 16 #hacer q provenga de la tile en custom data

@onready var body_holder: Node2D = $BodyHolder
@onready var camera_2d: Camera2D = $Camera2D

@onready var internal_state: BeingInternalState = $InternalState

signal load_tiles_around_me(coords: Vector2, chunk_size: Vector2i, uid: int)

@onready var body: AnimatedBodyPortion = $BodyHolder/Body
@onready var head: AnimatedBodyPortion = $BodyHolder/Head

func _ready():
	_connect_tile_map.rpc()

@rpc("call_local")
func _connect_tile_map():
	var tile_map: RustTileMap = get_parent()
	load_tiles_around_me.connect(tile_map.load_tiles_around)

#constructs for multiplayer too
func construct(preiniter: BeingStatePreIniter) -> void:
	if preiniter.sprite_body:
		body.construct(preiniter.sprite_body, preiniter.body_scale)
		if preiniter.sprite_head:
			head.construct(preiniter.sprite_head, preiniter.head_scale, preiniter.sprite_body.head_v_offset, preiniter.body_scale.z)
			
	internal_state.construct_from_seri.rpc(preiniter.internal_state.serialize())

var uncontrolled: bool = true

@rpc("call_local")
func give_control(peer_id: int) -> void:
	if internal_state._faction is PlayerFaction and internal_state._race is ControllableRace:
		uncontrolled = false
		set_multiplayer_authority(peer_id)
		if peer_id == multiplayer.get_unique_id() and internal_state._faction is PlayerFaction:
			camera_2d.make_current()

@rpc("call_local", "any_peer")
func take_control() -> void:
	if internal_state.faction is PlayerFaction and uncontrolled and internal_state.race is ControllableRace:
		uncontrolled = false
		set_multiplayer_authority(multiplayer.get_remote_sender_id())
		if multiplayer.get_unique_id() == multiplayer.get_remote_sender_id():
			camera_2d.make_current()

@rpc("call_local") 
func free_control() -> void: uncontrolled = true
		
var zoom_min = Vector2(0.05, 0.05); var zoom_max = Vector2(9999999, 9999999)		
		
func _input(event: InputEvent) -> void:
	if is_multiplayer_authority() and event.is_pressed():
		
		if event is InputEventMouseButton:
			if event.is_action("wheel_down"):
				camera_2d.zoom *= 0.9
			elif event.is_action("wheel_up"):
				camera_2d.zoom *= 1.1
			camera_2d.zoom = camera_2d.zoom.clamp(zoom_min, zoom_max)
			
		if GlobalData.debug and event.is_action("f1"):
			print((get_parent() as TileMap).local_to_map(position))

func _physics_process(delta: float) -> void:
	match [is_multiplayer_authority(), internal_state._faction is PlayerFaction, uncontrolled]:
		[false, ..]:
			return
		[_, false, _]:
			ai_control()
		[_, true, false]:
			_update_direction_axis_by_input(delta)
		[_, true, true]:
			owned_ai_control()
			
	_update_distance_moved()
	_update_body_state()
	
	_process_animation()

var distance_moved: float; var _previous_position: Vector2 = position
func _update_distance_moved() -> void:
	distance_moved = position.distance_to(_previous_position)
	_previous_position = position
	
func _update_body_state() -> void:
	if distance_moved > 1:
		_adjust_speed_scale.rpc(distance_moved, 1)
		_change_body_state.rpc(BodyState.JOG)
	elif distance_moved > 0.01:
		_adjust_speed_scale.rpc(distance_moved, 0.8)
		_change_body_state.rpc(BodyState.WALK)
	else:
		_change_body_state.rpc(BodyState.IDLE)
	

enum BodyState { IDLE, WALK, JOG }

var _body_state: BodyState = BodyState.IDLE
		
var _facing_direction: String = "down"

@rpc("call_local", "unreliable")
func _change_body_state(new_body_state: BodyState):
	_body_state = new_body_state
@rpc ("call_local", "unreliable")
func _adjust_speed_scale(distance_moved: float, factor: float):
	for body_part in body_holder.get_children():
		if body_part is AnimatedBodyPortion:
			body_part.speed_scale = distance_moved/factor
		
func owned_ai_control(): pass		
func ai_control(): pass
	
var _direction_axis: Vector2 = Vector2.ZERO
var _velocity: Vector2 = Vector2.ZERO

var distance_moved_since_load: float = 501
func _update_direction_axis_by_input(delta: float) -> void:
	
	_direction_axis = Input.get_vector("ui_left", "ui_right", "ui_up", "ui_down")
	
	apply_friction(friction, delta)
	
	if _direction_axis != Vector2.ZERO:
		_direction_axis = _direction_axis.normalized()
		_velocity += _direction_axis * acceleration * delta
		_velocity = _velocity.limit_length(internal_state.get_max_speed())
		_update_facing_direction()
	
	if not GlobalData.noclip:
		move_and_collide(_velocity*GlobalData.debug_walk_mult)
	else:
		position += _direction_axis * GlobalData.noclip_speed_mult
	
	distance_moved_since_load += distance_moved
	
	if distance_moved_since_load > 500:
		load_tiles_around_me.emit(position, Vector2i(192, 120), uid)#195, 120
		distance_moved_since_load = 0
		
func apply_friction(amount: float, delta: float):
	_velocity = _velocity.move_toward(Vector2.ZERO, amount * delta)
	
		
func _update_facing_direction() -> void:
	if abs(_direction_axis.x) > abs(_direction_axis.y): 
		_facing_direction = "left" if _direction_axis.x < 0 else "right"
	else: 
		_facing_direction = "up" if _direction_axis.y < 0 else "down"

# esto debería ser un componente?
func _process_animation() -> void:	
	_play_animation(str(BodyState.keys()[_body_state]).to_lower()
	 + "_" + _facing_direction)

func _play_animation(animation_name: String) -> void:	
	for body_part in body_holder.get_children():
		if body_part is AnimatedBodyPortion and body_part.sprite_frames:
			body_part._play_handled(animation_name)
			
func serialize() -> Dictionary:
	return {
		"direction": _facing_direction,
		"position": position,
		"state": internal_state.serialize()
	}
