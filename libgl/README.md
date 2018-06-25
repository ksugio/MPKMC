# Compile in Windows
Visual Studio Community and Python library are required to create MPGLKMC.dll (MPGLKMC.pyd).
Open MPGLKMC.sln and set include path and library path according to [Property Manager] - [Microsoft.cpp.Win32.user] - [VC++ Directory] - [Include Directory] and [Library Directory].
Example of include path is shown.

    C:\Python27\include
    C:\Python27\Lib\site-packages\numpy\core\include

Example of library path is shown.

    C:\Python27\libs
    C:\Python27\Lib\site-packages\numpy\core\lib

Build in Release mode and MPGLKMC.pyd is copied to python directory.

# Compile in Linux
Edit Makefile and execute make.

    vi Makefile
    make install

MPGLKMC.so is created and copied to python directory.

# References
## draw()
+ CLASS METHODS
  + atoms(kmc, cmp) : draw atoms
  + atoms region(kmc) : return atoms region for drawing
  + axis(kmc, (lx, ly, lz), dia) : draw axis
  + cluster(kmc, cmp, id) : draw a cluster registered in energy table  
  + cluster_region(kmc) : return cluster region for drawing
  + colormap_range(kmc, cmp) : set colormap range
  + frame(kmc) : draw frame
  + set_dia(id) : get diameter of spheres
  + set_disp(id) : get display of spheres
  + set_dia(id, dia) : set diameter of spheres
  + set_disp(id, disp) : set display of spheres
  + translate(kmc, x, y, z) : OpenGL translation
  + types(id) : return registered type
+ CLASS DATA
  + frame_color = (red, green, blue) : frame color
  + frame_width = width : frame width
  + kind = {0:type | 1:energy} : draw kind
  + ntypes : number of registered types
  + res = res : resolution of sphere
  + shift = (dx, dy, dz) : shift in drawing

## colormap()
+ CLASS METHODS
  + color() : set default color
  + draw() : draw colormap
  + grad_color(value) : get grad color
  + grayscale() : set default grayscale
  + set_grad_color(id, red, green, blue) : set grad color
  + set_label(id, label) : set label
  + set_step_color(id, red, green, blue) : set step color
  + step_color(id) : get step color
+ CLASS DATA
  + font_color = (red, green, blue) : font color
  + font_type = {0:10pt | 1:12pt | 2:18pt} : font type
  + mode = {0:step | 1:gradation} : colormap mode
  + ngrad = num : number of gradiation color
  + nscale = num : number of scale
  + nstep = num : number of step color
  + range = (min, max) : colormap range
  + size = (width, height) : colormap size
  + title = txt : colormap title

## model()
+ CLASS METHODS
  + fit_center(region) : fit center
  + fit_scale(region, aspect) : fit scale
  + get_angle() : get angle
  + get_dir() : get direction
  + init() : initialize matrix
  + inverse() : set inverse matrix
  + rot_x(angle) : rotate around X axis
  + rot_y(angle) : rotate around Y axis
  + rot_z(angle) : rotate around Z axis
  + set_angle(alpha, beta, gamma) : set angle
  + set_dir(x0, x1, x2, [z0, z1, z2]) : set direction
  + trans_x(dist) : translate to X direction
  + trans_y(dist) : translate to Y direction
  + trans_z(dist) : translate to Z direction
  + transform() : OpenGL transformation
  + zoom(scale) : zoom
+ CLASS DATA  
  + center = (cx, cy, cz) : center of rotation
  + mat = ((m00, m01, m02, m03), (...), (...), (...)) : transformation matrix
  + mat_inv = ((i00, i01, i02, i03), (...), (...), (...)) : inversed transformation matrix
  + scale = scale : scale

## scene()
+ CLASS METHODS
  + light_add(x, y, z, w) : add light
  + light_ambient(id, red, green, blue, alpha) : set light ambient
  + light_diffuse(id, red, green, blue, alpha) : set light diffuse
  + light_position(id, x, y, z, w) : set light position
  + light_specular(id, red, green, blue, alpha) : set light specular
  + resize(width, height) : resize window
  + setup() : setup scene
+ CLASS DATA
  + clear_color = (red, green, blue, alpha) : clear color
  + lookat = (ex, ey, ez, cx, cy, cz, ux, uy, yz) : viewpoint
  + mat_emission = (red, green, blue, alpha) : material emission
  + mat_shininess = shininess : material shininess
  + mat_specular = (red, green, blue, alpha) : material specular
  + proj = {0:frustum 1:ortho} : projection mode
  + zfar = z : zfar of viewing volume
  + znear = z : znear of viewing volume
