// disc_housing.scad — holds the disc + actuator; has the scoop window under the
// hopper, the drop-hole to the chute, the fixed wiper/scraper lip, and the IR
// sensor slot. (brief §8: disc_housing.scad)
//
// The disc (singulation_disc.scad) sits in the circular pocket of this housing,
// resting on the floor. The floor blocks pockets everywhere EXCEPT the drop-hole,
// so a pill rides in its pocket until it reaches the drop station and falls.
include <params.scad>

// Where, around the disc, things sit (degrees). Hopper scoops on one side; the
// drop-hole is offset so a scooped pill travels under the wiper before dropping.
scoop_angle = 90;    // hopper feeds here
drop_angle  = 270;   // pill falls here (opposite side)
wiper_angle = 160;   // scraper lip sits between scoop and drop

housing_od   = disc_d + 2*(disc_clear + wall);
cavity_d     = disc_d + 2*disc_clear;
cavity_h     = disc_thickness + 0.6;          // a hair over disc so it spins free
housing_h    = floor_thick + cavity_h;

module disc_cavity() {
  translate([0, 0, floor_thick])
    cylinder(h = cavity_h + 0.1, d = cavity_d);
}

module drop_hole() {
  // Through the floor at the drop station, tapering outward downward into the chute.
  translate([pocket_ring_r, 0, 0]) rotate([0,0,0])
    translate([0,0,-0.1])
      cylinder(h = floor_thick + 0.2, d1 = drop_hole_d + 2, d2 = drop_hole_d);
}

module sensor_slot() {
  // Horizontal slot across the drop path for the IR emitter/receiver beam.
  translate([pocket_ring_r, 0, floor_thick/2])
    cube([housing_od, sensor_slot_w, sensor_slot_w], center = true);
}

module wiper_lip() {
  // Fixed scraper, as a CANTILEVER ARM connected to the outer wall (so it prints as
  // one piece — a free-floating lip is unprintable). The arm spans radially from the
  // rim inward over the pocket path; its underside sits just above a SEATED pill
  // (which is flush with the disc top, since pocket depth ≈ pill thickness), so a
  // pill riding proud of its pocket is knocked back (anti double-stack).
  z_bottom = floor_thick + disc_thickness + 0.3;  // clears a seated pill, catches a proud one
  arm_h    = 3.0;
  inner_r  = pocket_ring_r - round_pocket_d*0.3;   // scraper tip reaches over the pocket
  outer_r  = housing_od/2;                          // embed into the outer wall
  arm_len  = outer_r - inner_r + wall;
  rotate([0, 0, wiper_angle])
    translate([(inner_r + outer_r)/2, 0, z_bottom + arm_h/2])
      cube([arm_len, 3.0, arm_h], center = true);
}

module housing_body() {
  difference() {
    cylinder(h = housing_h, d = housing_od);   // solid puck
    disc_cavity();                             // recess the disc sits in
    rotate([0,0,drop_angle]) drop_hole();      // pill exit
    rotate([0,0,drop_angle]) sensor_slot();    // IR beam across the exit
    // scoop window: open the cavity wall toward the hopper so pills can enter
    rotate([0,0,scoop_angle])
      translate([pocket_ring_r, 0, floor_thick + cavity_h/2])
        cube([round_pocket_d*1.8, round_pocket_d*1.8, cavity_h + 1], center = true);
  }
}

module disc_housing() {
  union() {
    housing_body();
    wiper_lip();
    // NOTE: actuator mount (SG90 pocket OR 28BYJ-48 boss) bolts to the underside.
    // Left as a labelled stub — its footprint depends on shaft_type and is added in
    // Phase C once the disc↔shaft coupling is validated on a printed test piece.
  }
}

disc_housing();

echo(str("housing_od=", housing_od, "  drop_angle=", drop_angle,
         "  wiper_gap=", wiper_gap));
