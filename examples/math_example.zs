ZetriScript [0:0:0]!

[0:0:0]: l1 = LINE([0:0:2],[0:0:1],0,6,1); goto [0:0:9]!
[0:0:9]: l2 = LINE([0:0:10],[0:0:1],0,7,1); goto [0:0:1]!

[0:0:1]: draw_h = allocSpace<Euclidean2D>(l1);
[0:0:2]: point (0, 0)!
[0:0:3]: point (0, 1)!
[0:0:4]: point (0, 2)!
[0:0:5]: point (1, 1)!
[0:0:6]: point (2, 0)!
[0:0:7]: point (2, 1)!
[0:0:8]: point (2, 2)!

[0:0:10]: draw_e = allocSpace<Euclidean2D>(l2);
[0:0:11]: point (4, 0)!
[0:0:12]: point (4, 1)!
[0:0:13]: point (4, 2)!
[0:0:14]: point (5, 0)!
[0:0:15]: point (5, 1)!
[0:0:16]: point (5, 2)!
[0:0:17]: point (6, 0)!

[0:0:18]: CALL draw_h!

// Define a non-spatial custom system
[0:0:20] {
    system NON_SPATIAL_SYSTEM:
        // Define specific commands and rules for this system
        command CALCULATE<x, y>:
            result = x + y!  // Simple calculation
            PRINT result!    // Output the result
        END

        command DATA_PROCESS<data>:
            // Process data without spatial context
            PROCESSED_DATA = process(data)
            PRINT PROCESSED_DATA!
        END
    END
}


ZetriScript