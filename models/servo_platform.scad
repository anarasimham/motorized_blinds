$fn=100;

coincident_offset = .1;

header_width = 56;
block_len = 38;
base_height = 5;
total_height = 9;

//wall_thickness = 5;

motor_width = 40.5;
motor_support_len = 18;
support_brace_height = 25;
motor_wall_thickness = 7.75;
motor_support_front_space = 8;
motor_screw_height = 4.3125;
motor_support_width = 18;

screw_bore_dia = 3.25;

difference() {
    cube([header_width,block_len,total_height]);
   
    //anchoring screws
    translate([motor_wall_thickness/2, motor_support_front_space/2, -coincident_offset]) cylinder(h=total_height+coincident_offset*2, r=screw_bore_dia/2-0.25); 
    translate([header_width-motor_wall_thickness/2, motor_support_front_space/2, -coincident_offset]) cylinder(h=total_height+coincident_offset*2, r=screw_bore_dia/2-0.25); 
    translate([motor_wall_thickness/2, block_len-motor_support_front_space/2, -coincident_offset]) cylinder(h=total_height+coincident_offset*2, r=screw_bore_dia/2-0.25); 
    translate([header_width-motor_wall_thickness/2, block_len-motor_support_front_space/2, -coincident_offset]) cylinder(h=total_height+coincident_offset*2, r=screw_bore_dia/2-0.25); 
    //translate([0*header_width+wall_thickness,-1*coincident_offset,-1*coincident_offset]) cube([header_width-2*wall_thickness,block_len+coincident_offset*2,total_height-wall_thickness+coincident_offset]);
}



/*
motor supports
*/
translate([header_width*0.5+motor_width/2,motor_support_front_space,total_height]) {
    difference() {
        cube([motor_wall_thickness, motor_support_len, support_brace_height]);
        translate([motor_screw_height,-coincident_offset,0.27*motor_support_width])
            rotate(a=-90, v=[1,0,0])
            cylinder(r=screw_bore_dia/2-0.25, h=motor_support_len+coincident_offset*2);
        translate([motor_screw_height,-coincident_offset,0.86*motor_support_width])
            rotate(a=-90, v=[1,0,0])
            cylinder(r=screw_bore_dia/2-0.25, h=motor_support_len+coincident_offset*2);
    }
}

translate([header_width*0.5-motor_width/2-motor_wall_thickness,motor_support_front_space,total_height]) {
    difference() {
        cube([motor_wall_thickness, motor_support_len, support_brace_height]);
        translate([motor_screw_height,-coincident_offset,0.27*motor_support_width])
            rotate(a=-90, v=[1,0,0])
            cylinder(r=screw_bore_dia/2-0.25, h=motor_support_len+coincident_offset*2);
        translate([motor_screw_height,-coincident_offset,0.86*motor_support_width])
            rotate(a=-90, v=[1,0,0])
            cylinder(r=screw_bore_dia/2-0.25, h=motor_support_len+coincident_offset*2);
    }
}