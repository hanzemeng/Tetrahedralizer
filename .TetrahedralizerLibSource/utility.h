#ifndef UTILITY
#define UTILITY

#include "program_data.h"

int double_to_int(double d);
int orient3d(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3);
int in_sphere(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4);
bool is_collinear(uint32_t p0, uint32_t p1, uint32_t p2);
bool vertex_in_inner_segment(uint32_t p0,uint32_t s0,uint32_t s1);
bool vertex_in_segment(uint32_t p0,uint32_t s0,uint32_t s1);
bool vertex_in_inner_triangle(uint32_t p0,uint32_t t0,uint32_t t1,uint32_t t2);
bool vertex_in_triangle(uint32_t p0,uint32_t t0,uint32_t t1,uint32_t t2);
bool inner_segment_cross_inner_triangle(uint32_t s0,uint32_t s1,uint32_t t0,uint32_t t1,uint32_t t2);
bool inner_segment_cross_triangle(uint32_t s0,uint32_t s1,uint32_t t0,uint32_t t1,uint32_t t2);
bool inner_segment_cross_inner_segment(uint32_t s0,uint32_t s1,uint32_t s2,uint32_t s3);

// let n = (x,y,z) be the normal of the triangle <t0,t1,t2>; let m = max(|x|, |y|, |z|).
// returns 0 if m == |x|; 1 if m == |y|; 2 if m == |z|
// This function may crash if <t0,t1,t2> is degenerate.
int max_component_in_triangle_normal(uint32_t t0,uint32_t t1,uint32_t t2); 
int orient3d_ignore_axis(uint32_t p0,uint32_t p1,uint32_t p2,int axis);
int orient3d_ignore_axis(const explicitPoint3D& pp0,uint32_t p1,uint32_t p2,int axis);


void sort_ints(uint32_t& i0, uint32_t& i1);
void sort_ints(uint32_t& i0, uint32_t& i1, uint32_t& i2);
void sort_ints(int& i0, int& i1);


void clear_queue(queue<uint32_t>& q);
void clear_stack(stack<uint32_t>& s);


void get_tetrahedron_face(uint32_t t, uint32_t i, uint32_t& f0,uint32_t& f1,uint32_t& f2);
void get_tetrahedron_face(uint32_t t, uint32_t& f0,uint32_t& f1,uint32_t& f2);
void get_tetrahedron_opposite_vertex(uint32_t t, uint32_t& p);
void get_tetrahedron_neighbor(uint32_t t, uint32_t i, uint32_t& n);


#endif
