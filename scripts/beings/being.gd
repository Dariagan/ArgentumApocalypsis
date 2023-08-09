extends CharacterBody2D
class_name Being
#así añadimos nueva funcionalidad en base a composición en vez de a herencia (sostenible a largo plazo) 

@export var speed: float = 3
@export var acceleration = 30

@export var friction = 16 #hacer q provenga del piso

@onready var body_holder: Node2D = $BodyHolder
@onready var camera_2d: Camera2D = $Camera2D

enum BodyState { IDLE, WALK, JOG }

var body_state: BodyState = BodyState.IDLE
		
var _facing_direction: String = "down"

var being_data: BeingPersonalData = BeingPersonalData.new()

@onready var body: AnimatedBodyPortion = $BodyHolder/Body
@onready var head: AnimatedBodyPortion = $BodyHolder/Head
func construct(data: BeingSpawnData) -> void:
	if data.body_i > -1:
		var body_sprite_data: BodySpriteData = data.race.body_sprites_datas[data.body_i]
		body.construct(body_sprite_data, data.body_scale)
		if data.head_i > -1:
			var head_sprite_data: SpriteData = data.race.head_sprites_datas[data.head_i]
			head.construct(head_sprite_data, data.head_scale, body_sprite_data.head_v_offset, data.body_scale.z)
	
	
	
@rpc("call_local")
func construct_being_data(data: Dictionary):
	pass


var uncontrolled: bool = true

@rpc("call_local")
func give_control(peer_id: int) -> void:
	if being_data.faction is PlayerFaction and being_data.race is ControllableRace:
		uncontrolled = false
		take_camera.rpc_id(peer_id)
		set_multiplayer_authority(peer_id)
		
@rpc func take_camera(): if being_data.faction is PlayerFaction: camera_2d.make_current()

@rpc("call_local", "any_peer")
func take_control() -> void:
	if being_data.faction is PlayerFaction and uncontrolled and being_data.race is ControllableRace:
		uncontrolled = false
		set_multiplayer_authority(multiplayer.get_remote_sender_id())
		if multiplayer.get_unique_id() == multiplayer.get_remote_sender_id():
			#camera_2d.enabled = true
			camera_2d.make_current()

@rpc("call_local") 
func free_control() -> void: 
	uncontrolled = true
	#if multiplayer.get_unique_id() == multiplayer.get_remote_sender_id():
		#camera_2d.enabled = false


func _physics_process(delta: float) -> void:
	
	match [is_multiplayer_authority(), being_data.faction is PlayerFaction, uncontrolled]:
		[false, ..]:
			return
		[_, false, _]:
			ai_control()
		[_, true, false]:
			move_by_input(delta)
		[_, true, true]:
			owned_ai_control()
		
func owned_ai_control(): pass		
func ai_control(): pass
	
var _input_axis: Vector2 = Vector2.ZERO
var _velocity: Vector2 = Vector2.ZERO
var previous_position: Vector2 = position
func move_by_input(delta: float) -> void:
	
	_input_axis = Input.get_vector("ui_left", "ui_right", "ui_up", "ui_down")
	
	body_state = BodyState.IDLE
	
	apply_friction(friction, delta)
	
	if _input_axis != Vector2.ZERO:
		_input_axis = _input_axis.normalized()
		_velocity += _input_axis * acceleration * delta
		_velocity = _velocity.limit_length(speed)
		_update_facing_direction()
	
	move_and_collide(_velocity)
	
	var distance_moved: float = position.distance_to(previous_position)

	if distance_moved > 1:
		for body_part in body_holder.get_children():
			body_part.speed_scale = distance_moved/1
		body_state = BodyState.JOG
		
	elif distance_moved > 0.01:
		for body_part in body_holder.get_children():
			body_part.speed_scale = distance_moved/0.8
		body_state = BodyState.WALK
	else:
		body_state = BodyState.IDLE
	
	_process_animation()
	
	previous_position = position
	
func apply_friction(amount: float, delta: float):
	var real_amount: float = amount * delta
	
	# _velocity = _velocity.move_toward(Vector2.ZERO, real_amount)
	if _velocity.length() > real_amount:
		_velocity -= _velocity.normalized() * real_amount
	else:
		_velocity = Vector2.ZERO
		
func _update_facing_direction() -> void:
	if abs(_input_axis.x) > abs(_input_axis.y): 
		_facing_direction = "left" if _input_axis.x < 0 else "right"
	else: 
		_facing_direction = "up" if _input_axis.y < 0 else "down"

# esto debería ser un componente
func _process_animation() -> void:	
	_play_animation(str(BodyState.keys()[body_state]).to_lower()
	 + "_" + _facing_direction)

func _play_animation(animation_name: String) -> void:	
	for body_part in body_holder.get_children():
		if body_part.sprite_frames:
			body_part._play_handled(animation_name)
