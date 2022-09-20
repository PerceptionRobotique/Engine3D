//POINT VERTEX
//#version 330 core
#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

attribute vec3 in_vertex;
attribute vec3 in_color;
attribute float in_intensity;

uniform mat4 wMo;
uniform mat4 cMw;
uniform mat4 iMc;
uniform float scale;

uniform float opacity;
uniform bool showIntensity;
uniform bool customColor;
uniform float R;
uniform float G;
uniform float B;
uniform float pointSize;

uniform bool equirectangular;

varying vec4 color;

void main()
{
    vec3 vertex = in_vertex * scale;
    gl_PointSize = pointSize;

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

    if (customColor) color = vec4(R,G,B, opacity);
    else
    {
        if(showIntensity)
            color = vec4(in_intensity, in_intensity, in_intensity, opacity);
        else
            color = vec4(in_color, opacity);
    }
}
