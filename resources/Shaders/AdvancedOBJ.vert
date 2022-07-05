//#version 330
#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

attribute vec3 in_vertex;
attribute vec3 in_normal;
attribute vec2 in_uv;

uniform mat4 wMo;
uniform mat4 cMw;
uniform mat4 iMc;
uniform float scale;

uniform bool equirectangular;

varying vec3 vertex;
varying vec3 normal;
varying vec2 texCoord;

void main()
{
    vertex = in_vertex * scale;
    if(equirectangular)
    {
        // changement de repere objet -> camera
        vec4 cP = cMw * wMo * vec4(vertex, 1.0);
        float fact = length(cP.xyz);

        // projection spherique
        cP.xyz /= fact;

        // codage azimuth et elevation
        cP.x = atan(cP.z, cP.x);
        cP.y = -2.0*acos(cP.y)+3.14;
        cP.z = -3.14;

        //"projection" (division par Z)
        gl_Position = iMc * cP;

        //cette ligne et suivante pour mettre rho code a la zbuffer dans le zbuffer
        cP = iMc * vec4(0.,0.,-fact,1.);

        gl_Position.z = gl_Position.w*cP.z/cP.w;
    }
    else
        gl_Position = iMc * cMw * wMo * vec4(vertex, 1.0);

    vertex = in_vertex;
    normal = in_normal;
    texCoord = in_uv;
}
