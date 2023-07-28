extends Resource
class_name DamageDef

enum Mobility { NONE, MOBILIZES, CONFUSES, IMMOBILIZES, PARALYZES }
enum Invisibility { NONE, REMOVES, GIVES }

@export_category("Armor Reduction")

@export_range(0, 2) var blunt_armor_reduction_ratio: float
# tanto-por-uno del daño hecho que se resta a la armor antes de que esta reste tu damage
#puede ser más q uno

# PERO, la penetración extra no causa daño extra

#solo si se supera esta armor se causa bleeding
@export_range(0, 2) var sharp_armor_reduction_ratio: float


@export_range(0, 2) var fire_armor_reduction_ratio: float
@export_range(0, 2) var magical_armor_reduction_ratio: float

@export_category("Effects")

@export var visibility_effect: Invisibility
@export var visibility_effect_duration: float

@export var mobility_effect: Mobility
@export var mobility_effect_duration: float

@export_range(-1, 1) var blindness: float

# can be negative or positive
@export_range(-15, 15) var speed: float



@export_category("Lasting damage")

#debería ser small
@export var venom: float # se podria hacer una clase poison, y q cada uno requiera un trtamiento especial

@export_range(0, 1) var venom_chance: float


@export var poison_duration: float

@export_range(0, 1) var wound_infection_chance_increase: float

@export_range(0, 0.3) var incineration_ratio_on_penetration: float
#hacer q la armor reduction dure??

@export_range(0, 0.3) var bleeding_ratio_on_penetration: float

