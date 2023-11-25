extends RefCounted 
#BeingRequiredInitializationData
class_name BeingReqInitData

var name: String 
var head_scale: Vector3 = Vector3.ONE
var body_scale: Vector3 = Vector3.ONE

var sprite_head: SpriteData
var sprite_body: BodySpriteData

var chosen_extra_sprites: Array[int] = []

var extra_health_multiplier: float = 1

var internal_state: BeingInternalState
	
# inicializar las variables con .nombre_variable = algo
	
func construct(being_birth_dict: Dictionary) -> void:
	var sex: Enums.Sex
	var race: BasicRace
	var klass: Class
	var faction: Faction
	var followers: Array[UncontrollableRace]  
	var result
	result = handle_key("name", being_birth_dict)
	if result: name = result; result = null
	
	result = handle_key("eh", being_birth_dict)
	if result: extra_health_multiplier = result; result = null
		
	#result = handle_key("level", being_birth_dict)
	#if result: level = result; result = null
		
	var race_id: String = being_birth_dict["race"]
	if race_id.begins_with("controllable"):
		race = handle_key("race", being_birth_dict, GlobalData.controllable_races)
	else:
		race = handle_key("race", being_birth_dict, GlobalData.uncontrollable_races)
		
	klass = handle_key("klass", being_birth_dict, GlobalData.classes)
	
	faction = handle_key("fac", being_birth_dict, GameData.factions)
		
	if being_birth_dict.has("followers"):
		# BUG, ARREGLAR. DICE CLASSES AHÍ. HAY Q ARREGLAR ETO
		followers = GlobalData.classes[being_birth_dict["followers"]]
			
	sprite_head = handle_key("head", being_birth_dict, race.head_sprites_datas)
			
	sprite_body = handle_key("body", being_birth_dict, race.body_sprites_datas) as BodySpriteData
	
	result = handle_key("head_scale", being_birth_dict)
	if result: head_scale = result; result = null
	
	result = handle_key("body_scale", being_birth_dict)
	if result: body_scale = result; result = null
	
	internal_state = BeingInternalState.new()
	internal_state.construct_locally(sex, race, faction, null, klass)
	
func construct_from_serialized(serialized_being_spawn_data: Dictionary) -> void:
	pass

func serialize() -> Dictionary:
	var dict: Dictionary = {
		"name": name,
		"head_scale": head_scale,
		"body_scale": body_scale,
		"head": sprite_head.id,
		"body": sprite_body.id,
		"state": internal_state.serialize(),
		"eh": extra_health_multiplier
	}
	#dict["followers"] = get_array_of_ids(followers)
	
	return dict

#NO IMPLEMENTAR ESTA FUNCIÓN, PERO IMPLEMENTAR LA IDEA DE CARGAR STARTER CHARACTERS ASÍ NO PERDÉS TIEMPO RE-CREÁNDOLOS EN CADA LOBBY
func _construct_from_saved_starter_character(starter_character: Resource) -> void:
	pass

func get_array_of_ids(array_of_objects: Array) ->  Array:
	var array_ids: Array = []
	for o in array_of_objects:
		array_ids.push_back(o.id)
	return array_ids

func handle_key(key: String, being_birth_dict: Dictionary, data_structure = null):
	if being_birth_dict.has(key):
		if data_structure is Dictionary and data_structure.keys().size() > 0:
			if not (being_birth_dict[key] as String).ends_with("random"):
				return data_structure[being_birth_dict[key]]
			else:
				return get_random(data_structure.values())
		elif data_structure is Array and data_structure.size() > 0:
			for item in data_structure:
				if being_birth_dict[key] == item.id:
					return item
			return get_random(data_structure)
		else: 
			return being_birth_dict[key]
	else:
		print("key %s not found" % key)

func get_random(list_for_random: Array):
	if list_for_random.size() > 0: return list_for_random[randi() % list_for_random.size()]

