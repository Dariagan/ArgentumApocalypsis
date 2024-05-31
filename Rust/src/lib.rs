use godot::prelude::*;

struct ArgentumExtension;


#[gdextension]
unsafe impl ExtensionLibrary for ArgentumExtension {}

use godot::engine::Node2D;

#[derive(GodotClass)]
#[class(base=Node2D)]
struct Test{

    node2d: Base<Node2D>
}


#[godot_api]
impl INode2D for Test{
    fn init(node2d: Base<Node2D>) -> Self{
        godot_print!("hello");
    Self{ node2d }
    }
}


