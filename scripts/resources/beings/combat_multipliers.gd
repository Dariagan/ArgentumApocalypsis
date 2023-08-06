extends Resource
class_name CombatMultipliers

@export var max_health: float = 1
@export var max_speed: float = 1
@export var strength: float = 1
@export var stamina: float = 1

@export var sneaking_proficiency: float = 1
@export var incoming_damage_types_multipliers: Dictionary = {"rellenar": 1, "rellenar2": 1}
 #key: id of the damage type. value: value the incoming damage of the specified tyoe is multiplied by
@export var war_cry_strength: float = 1

@export_category("Melee Multipliers")
@export var global_melee_damage: float = 1
@export var global_melee_attack_speed: float = 1
@export var backstab_damage: float = 1
@export var body_attack_damage: float = 1
@export var body_attack_attack_speed: float = 1
@export var two_handed_damage: float = 1
@export var two_handed_attack_speed: float = 1
@export var two_handed_swing_damage: float = 1
@export var two_handed_swing_attack_speed: float = 1
@export var two_handed_stab_damage: float = 1
@export var two_handed_stab_attack_speed: float = 1
@export var one_handed_damage: float = 1
@export var one_handed_attack_speed: float = 1
@export var one_handed_swing_damage: float = 1
@export var one_handed_swing_speed: float = 1
@export var one_handed_stab_damage: float = 1
@export var one_handed_stab_speed: float = 1

@export var shield_proficiency: float = 1

@export_category("Ranged Multipliers")
@export var global_ranged_damage: float = 1
@export var global_ranged_attack_speed: float = 1
@export var global_range: float = 1
@export var throwable_damage: float = 1
@export var throwable_attack_speed: float = 1
@export var throwable_range: float = 1
@export var bow_damage: float = 1
@export var bow_attack_speed: float = 1
@export var bow_range: float = 1
@export var crossbow_arquebus_attack_speed: float = 1
@export var crossbow_arquebus_range: float = 1

@export_category("Magic Multipliers")
@export var global_spells_effect: float = 1
@export var healing_spells_effect: float = 1 #hacer los regeneration spells den más health x mana gasta, pero son mucho mas lentos
@export var spells_damage: float = 1
@export var mana_regeneration_rate: float = 1
