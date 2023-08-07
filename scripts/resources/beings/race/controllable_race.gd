extends BasicRace
class_name ControllableRace

@export var id: String = "controllable_"

@export var available_classes: Array[Class]

@export_category("Work Multipliers")

@export_range(0, 3) var global_learning_multiplier: float = 1
@export_range(0, 3) var smithing_learning_multiplier: float = 1
@export_range(0, 3) var manual_labor_multiplier: float = 1
@export_range(0, 3) var research_multiplier: float = 1
@export_range(0, 3) var trade_proficiency: float = 1
