ZetriScript (
    Decl: [1:1:1]!
    Exec: [-1:-1:-1]!
)

[1:1:1] {
    P1 = Plane<Equation>(0, 0, 1, -10);
    goto [0:0:1]!
}

[0:0:1] {
    system(a) Gravity = alloc_space(P1);
    goto [0:0:2]!
}

[0:0:2] {
    L1 = Line<0, 10, 1>([0:0:10], [1:0:0]);
    goto [0:0:3]!
}

[0:0:3] {
    command<x, y> fall = alloc_space(L1);
    goto [0:0:10]!
}

[0:0:10] {
    y = y - y * a;
    goto [0:0:4]!
}

[0:0:4] {
    L2 = Line<0, 10, 1>([0:1:10], [1:0:0]);
    goto [0:0:5]!
}

[0:0:5] {
    command(x, y) display = alloc_space(L2);
    goto [0:1:10];
}

[1:1:1]: P1 = Plane<Equation>(0,0,1,-10); goto [0:0:1]!
[0:0:1]: system(a) Gravity = alloc_space(P1); goto [0:0:2]!
[0:0:2]: L1 = Line<0, 10, 1>([0:0:10], [1:0:0]); goto [0:0:3]!
[0:0:3]: command<x, y> fall = alloc_space(L1); goto [0:0:10]!

[0:0:10]: y = y - y * a; goto [0:0:4]!
[0:0:4]: L2 = Line<0, 10, 1>([0:1:10], [1:0:0]); goto [0:0:5]!
[0:0:5]: command<x, y> display = alloc_space(L2); goto [0:1:10]!
[0:1:10]: Point<Euclidean>(x, y)! goto [0:0:6]!

[-1:-1:-1]: P = Point<Gravity>(5, 5); goto [-2:-1:-1]!
[-2:-1:-1]: P.fall! goto [-3:-1:-1]!
[-3:-1:-1]: P.display! goto [-1:-1:-1]!

ZetriScript