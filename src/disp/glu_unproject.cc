// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/*
This code is derived from the MESA3D project:
 http://www.mesa3d.org/relnotes/10.1.3.html
 
 Modified by F. Nedelec
 
 This code should be replaced by GLM
*/

#include <cstdio>


#define FLOAT GLdouble


void mulMatrixVec(const FLOAT matrix[16], const FLOAT in[4], FLOAT out[4])
{
    for (int i=0; i<4; i++)
        out[i] = in[0] * matrix[0*4+i] + in[1] * matrix[1*4+i]
               + in[2] * matrix[2*4+i] + in[3] * matrix[3*4+i];
}


// Invert 3x3 matrix.
int invMatrix3(const FLOAT m[9], FLOAT inv[9])
{
    FLOAT det = m[0]*m[4]*m[8] + m[2]*m[3]*m[7] + m[1]*m[5]*m[6]
              - m[2]*m[4]*m[6] - m[1]*m[3]*m[8] - m[0]*m[5]*m[7];
    
    if ( det != 0 )
    {
        det = 1.0 / det;
        inv[0] = ( m[4]*m[8] - m[5]*m[7] ) * det;
        inv[3] = ( m[5]*m[6] - m[3]*m[8] ) * det;
        inv[6] = ( m[3]*m[7] - m[4]*m[6] ) * det;
        inv[1] = ( m[2]*m[7] - m[1]*m[8] ) * det;
        inv[4] = ( m[0]*m[8] - m[2]*m[6] ) * det;
        inv[7] = ( m[1]*m[6] - m[0]*m[9] ) * det;
        inv[2] = ( m[1]*m[5] - m[2]*m[4] ) * det;
        inv[5] = ( m[2]*m[3] - m[0]*m[5] ) * det;
        inv[8] = ( m[0]*m[4] - m[1]*m[3] ) * det;
        return GL_TRUE;
    }
    return GL_FALSE;
}

/*
 ** Invert 4x4 matrix.
 ** Contributed by David Moore (See Mesa bug #6748)
 */
int invMatrix4(const FLOAT m[16], FLOAT inv[16])
{
    inv[ 0] =   m[ 5]*m[10]*m[15] - m[ 5]*m[11]*m[14] - m[ 9]*m[ 6]*m[15]
              + m[ 9]*m[ 7]*m[14] + m[13]*m[ 6]*m[11] - m[13]*m[ 7]*m[10];
    inv[ 4] = - m[ 4]*m[10]*m[15] + m[ 4]*m[11]*m[14] + m[ 8]*m[ 6]*m[15]
              - m[ 8]*m[ 7]*m[14] - m[12]*m[ 6]*m[11] + m[12]*m[ 7]*m[10];
    inv[ 8] =   m[ 4]*m[ 9]*m[15] - m[ 4]*m[11]*m[13] - m[ 8]*m[ 5]*m[15]
              + m[ 8]*m[ 7]*m[13] + m[12]*m[ 5]*m[11] - m[12]*m[ 7]*m[ 9];
    inv[12] = - m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
              - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    inv[ 1] = - m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
              - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv[ 5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
              + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv[ 9] = - m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
              - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv[13] =   m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
              + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    inv[ 2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
              + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
    inv[ 6] = - m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
              - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
    inv[10] =   m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
              + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
    inv[14] = - m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
              - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
    inv[ 3] = - m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
              - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
    inv[ 7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
              + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
    inv[11] =  -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
              - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
    inv[15] =   m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
              + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];
    
    FLOAT det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    
    if (det == 0)
        return GL_FALSE;
    
    det = 1.0 / det;
    
    for (int i = 0; i < 16; i++)
        inv[i] = inv[i] * det;
    
    return GL_TRUE;
}


void mulMatrices(const FLOAT a[16], const FLOAT b[16], FLOAT r[16])
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            r[i*4+j] = a[i*4+0]*b[0*4+j] + a[i*4+1]*b[1*4+j]
                     + a[i*4+2]*b[2*4+j] + a[i*4+3]*b[3*4+j];
        }
    }
}


/*
 We use a different name to avoid a possible name collision with GLUT
 */
GLint myUnproject(FLOAT winx, FLOAT winy, FLOAT winz,
                   const FLOAT modelMatrix[16],
                   const FLOAT projMatrix[16],
                   const GLint viewport[4],
                   FLOAT *objx, FLOAT *objy, FLOAT *objz)
{
    FLOAT mat[16];
    FLOAT inv[16];
    FLOAT in[4];
    FLOAT out[4];
    
    mulMatrices(modelMatrix, projMatrix, mat);
    
    if (!invMatrix4(mat, inv))
        return GL_FALSE;
    
    in[0]=winx;
    in[1]=winy;
    in[2]=winz;
    in[3]=1.0;
    
    /* Map x and y from window coordinates */
    in[0] = (in[0] - viewport[0]) / viewport[2];
    in[1] = (in[1] - viewport[1]) / viewport[3];
    
    /* Map to range -1 to 1 */
    in[0] = in[0] * 2 - 1;
    in[1] = in[1] * 2 - 1;
    in[2] = in[2] * 2 - 1;
    
    mulMatrixVec(inv, in, out);
    if (out[3] == 0.0)
        return GL_FALSE;

    *objx = out[0] / out[3];
    *objy = out[1] / out[3];
    *objz = out[2] / out[3];
    return GL_TRUE;
}


#undef FLOAT

