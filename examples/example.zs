// memory: 3D memory with depth like a tree node
[0:0:1] << system Gravity(a) {
    [0:0:2] << command fall(x, y) { // automatically position [0:0:2] will be allocated inside position [0:0:1]
        [0:0:3] << y = y - y * a; // automatically position [0:0:3] will be allocated inside position [0:0:2]
    }

    [0:0:4] << command display(x, y) { // automatically position [0:0:4] will be allocated inside position [0:0:1]
        [0:0:5] << P = Point<Euclidean>(x, y); // automatically position [0:0:5] will be allocated inside position [0:0:4]
        [0:0:6] << P.display! // automatically position [0:0:6] will be allocated inside position [0:0:4]
    }
}
// command allocation at position
[0:0:0] << P1 = Point<Gravity>(5, 5);
[0:0:10] << P1.fall();
[0:0:11] << P1.display();

-MAIN- {
    recall [0:0:1];
    recall [0:0:0];
    recall [0:0:10];
}
