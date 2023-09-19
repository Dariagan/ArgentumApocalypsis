using Godot;
using Godot.Collections;
using System;
using System.Collections.Generic;
using System.Diagnostics;

public partial class ProceduralGenerator: Resource
{
    [Export]
    private CSharpScript ShapeGenerator;

    [Export]
    private GDScript TilePicker;

    public string[,] Generate(string[,] worldMatrix, Vector2I generationCenter, Vector2I generationSize, Dictionary data = null, int seed = 0)
    {
        Debug.Assert(worldMatrix.GetLength(0) >= generationSize.X && worldMatrix.GetLength(1) >= generationSize.Y);

        return ((ShapeGenerator)ShapeGenerator.New()).Generate(TilePicker, worldMatrix, generationCenter, generationSize, data, seed);
    }
}
