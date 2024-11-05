-- The dictionary for shapes

-- If you need extra shapes, just put the shape file (Shape.lua) in
-- the "Shapes" folder in the plugin directory and add the Category
-- or shape name in the dictionary

local _M = {
    [1] = {
        name = "Geometric-2D",
        shapes = {
            [1] = { name = "Arrowhead", shapeName = "arrowHead" }, -- here "name" parameter is seen in dialog as option, so it should be Clearly readable
            [2] = { name = "Scalene Triangle", shapeName = "scaleneTriangle" },
            [3] = { name = "Equilateral Triangle", shapeName = "equilateralTriangle" },
            [4] = { name = "Right Angle Triangle", shapeName = "rightAngleTriangle" },
            [5] = { name = "Parallelogram", shapeName = "Parallelogram" },
            [6] = { name = "Hexagon", shapeName = "hexagon" },
            --  [7] = { name = "My Shape", shapeName = "myShape" },  -- Add your won shape like this
        },
    },
    [2] = {
        name = "Geometric-3D",
        shapes = {
            [1] = { name = "Cube", shapeName = "cube" },
            [2] = { name = "Cylinder", shapeName = "cylinder" },
            [3] = { name = "Sphere", shapeName = "sphere" },
            [4] = { name = "Hemisphere", shapeName = "hemisphere" },
            [5] = { name = "Cone", shapeName = "cone" },
            [6] = { name = "Dish", shapeName = "dish" },
        },
    },
    [3] = {
        name = "Axis System",
        shapes = {
            [1] = { name = "1st Quadrant", shapeName = "1st_Quad" },
            [2] = { name = "1st & 2nd Quadrants", shapeName = "1st_and_2nd_Quad" },
            [3] = { name = "1st & 4th Quadrants", shapeName = "1st_and_4th_Quad" },
            [4] = { name = "Full Axis", shapeName = "full_Axis" },
            [5] = { name = "Full Axis with Grids", shapeName = "full_with_Grids" },
        },
    },
    [4] = {
        name = "Graphs",
        shapes = {
            [1] = { name = "Sin x", shapeName = "sinx" },
            [2] = { name = "Cos x", shapeName = "cosx" },
            [3] = { name = "(sin x)^2", shapeName = "sinSqx" },
            [4] = { name = "(cos x)^2", shapeName = "cosSqx" },
            [5] = { name = "| sin x |", shapeName = "mod_sinx" },
            [6] = { name = "| cos x |", shapeName = "mod_cosx" },
        },
    },
    [5] = {
        name = "Physics-Diagrams",
        shapes = {
            [1] = { name = "Spring", shapeName = "spring" },
            [2] = { name = "Immovable Support", shapeName = "immuvableSupport" },
            [3] = { name = "Right Hand Rule", shapeName = "rightHandRule" },
            [4] = { name = "Vertical Circle", shapeName = "verticalCircle" },
            [5] = { name = "Venturi Meter", shapeName = "venturiMeter" },
            [6] = { name = "Cyclotron", shapeName = "cyclotron" },
            [7] = { name = "Car", shapeName = "car" },
            [8] = { name = "Organ Pipes", shapeName = "organPipes" },
        },
    },
    --    [6] = {
    --      name = "My Shape Category",
    --      shapes = {
    --          [1] = { name = "My Shape1", shapeName = "myShape1" }, -- here "name" parameter is seen in dialog as option, so it should be Clearly readable
    --          [2] = { name = "My Shape2", shapeName = "myShape2" },
    --          [3] = { name = "My Shape3", shapeName = "myShape3" },
    --          [4] = { name = "My Shape4", shapeName = "myShape4" },
    --      --  [5] = { name = "My Shape5", shapeName = "myShape5" },  -- Add your won shape like this
    --      },
    --
    --  }
} -- dont forget this bracket

return _M
