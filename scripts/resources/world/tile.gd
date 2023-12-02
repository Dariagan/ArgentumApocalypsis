extends Resource
class_name Tile

# NO MORE THAN 31 CHARACTERS
@export var id: String
	
@export var layer: int = 0
@export var source_id: int
@export var origin_position:Vector2i = Vector2i.ZERO
@export var flipped_at_random: bool = false

@export var random_scale_range: Vector4 # TODO tal vez es mejor hacer los bushes y trees escenas para poder hacer esto
#borrar la linea de arriba desp

@export var modulo_tiling_area: Vector2i = Vector2i(1, 1)
@export var alt_id: int

func get_data() -> Dictionary:
	
	if id == "":
		printerr("id is empty for tile")
		return {}
	if source_id == 0:
		printerr("source id is empty for tile")
		return {}
	
	var ID_LENGTH: int = id.length()

	if ID_LENGTH > GlobalData.CPP_BUFFER_LENGTH_MINUS_ONE:
		var original_id: String = id
		id = id.substr(ID_LENGTH % GlobalData.CPP_BUFFER_LENGTH_MINUS_ONE)
		id[0] = '#'
		printerr("tile.gd: using modified substring for tile_id '%s' -> '%s'" % [original_id, id])
		
	return {
		"id" = id, 
		"layer" = layer, 
		"source_id" = source_id,
		"fr" = flipped_at_random,
		"op" = origin_position,
		"ma" = modulo_tiling_area,
		"alt_id" = alt_id
		}
