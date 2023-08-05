extends VBoxContainer

@onready var race_menu_button: MenuButton = $RaceMenuButton
@onready var sex_menu_button: MenuButton = $SexMenuButton
@onready var head_menu_button: MenuButton = $HeadMenuButton
@onready var class_menu_button: MenuButton = $ClassMenuButton
@onready var follower_menu_button: MenuButton = $FollowerMenuButton

@onready var global_game_data = get_node("/root/GlobalGameData")

var _found_races: Array
var _current_race: ControllableRace
var _current_sex: GlobalEnums.Sex
var _current_head: HeadSpriteData
var _current_class: Class
var _current_follower: UncontrollableRace



# MULTIPLICAR LA FONT SIZE POR LA WIDTH DIVIDIDA ENTRE ALGUNA CONSTANTE? ASÍ SE LE CAMBIA EL TAMAÑO DINÁMICAMENTE
func _ready() -> void:
	_found_races = global_game_data.controllable_races.values()
	var race_menu_popup = race_menu_button.get_popup()
	_update_popup_menu(race_menu_popup, _found_races)
	race_menu_popup.id_pressed.connect(_on_race_selected)
	sex_menu_button.get_popup().id_pressed.connect(_on_sex_selected)
	head_menu_button.get_popup().id_pressed.connect(_on_head_selected)
	class_menu_button.get_popup().id_pressed.connect(_on_class_selected)
	follower_menu_button.get_popup().id_pressed.connect(_on_follower_selected)

func _on_race_selected(id: int):
	_current_race = _found_races[id]
	
	if _current_class and not _current_class in _current_race.available_classes:
		class_menu_button.text = "Class: Not picked"
		_current_class = null
		
	_setup_sex_menu_popup(_current_race)
	
	race_menu_button.text = "Race: %s" % _current_race.name
	
	if _current_race and _current_sex > 0:
		_setup_head_menu_popup()
	
	_update_popup_menu(class_menu_button.get_popup(), _current_race.available_classes)
	#selected_race.emit(_current_race)

func _on_sex_selected(id: int):
	_current_sex = id as GlobalEnums.Sex
	sex_menu_button.text = "Sex: %s" % str(GlobalEnums.Sex.keys()[id - 1])
	
	if _current_race and _current_sex > 0:
		_setup_head_menu_popup()
	
func _on_head_selected(id: int):
	_current_head = _current_race.head_sprites[id]
	head_menu_button.text = " "
	head_menu_button.icon = _current_head.animation.get_frame_texture("idle_down", 0)

func _on_class_selected(id: int):
	_current_class = _current_race.available_classes[id]
	
	if _current_follower and not _current_follower in _current_class.available_followers:
		follower_menu_button.text = "Follower: Not picked"
		_current_follower = null
	
	class_menu_button.text = "Class: %s" % _current_class.name
	_update_popup_menu(follower_menu_button.get_popup(), _current_class.available_followers)
	
func _on_follower_selected(id: int):
	_current_follower = _current_class.available_followers[id]
	follower_menu_button.text = "Follower: %s" % _current_follower.name
	
	
func _update_popup_menu(popup_menu: PopupMenu, items: Array):
	popup_menu.clear()
	var i: int = 0
	for item in items:
		if item.icon:
			popup_menu.add_icon_item(item.icon, item.name, i)
		elif item.head_sprites and item.head_sprites.size() >= 1 and item.head_sprites[0].animation:
			popup_menu.add_icon_item(item.head_sprites[0].animation.get_frame_texture("idle_down", 0), item.name, i)
		else:
			popup_menu.add_item(item.name, i)
		i += 1

func _setup_sex_menu_popup(current_race: ControllableRace):
	var popup: PopupMenu = sex_menu_button.get_popup()
	popup.clear()
	if current_race.males_females_ratio == 1:
		popup.add_item("Male", 1)
		_current_sex = GlobalEnums.Sex.MALE
		sex_menu_button.text = "Sex: Male"
	elif current_race.males_females_ratio == 0:
		popup.add_item("Female", 2)
		_current_sex = GlobalEnums.Sex.FEMALE
		sex_menu_button.text = "Sex: Female"
	else:
		popup.add_item("Male", 1)
		popup.add_item("Female", 2)

func _setup_head_menu_popup():
	var popup: PopupMenu = head_menu_button.get_popup()
	popup.clear()
	var i: int = 0
	for head_sprite in _current_race.head_sprites:
		popup.add_icon_item(head_sprite.animation.get_frame_texture("idle_down", 0), "", i)
		i += 1
	
	if _current_head and not _current_head in _current_race.head_sprites:
		head_menu_button.text = "Head: Not picked"
		_current_head = null
