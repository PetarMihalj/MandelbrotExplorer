from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL.shaders import *
import glm
import time

import numpy
from PIL import Image

numpy.random.seed(120)


def time_s():
    return time.time_ns()/float(1E9)


class ParticleApp:
    vertex_shader = """
        # version 450 core
        layout (location = 0) in vec3 pos;

        uniform float time;
        uniform vec3 speed_constant;
        uniform vec3 acc_constant;

        void main()
        {
            vec3 newpos = pos + time*speed_constant + 0.5 * time*time*acc_constant;
            gl_Position = vec4(newpos, 1.0);
        }
    """

    geometry_shader = """
        # version 450 core
        layout (points) in;
        layout (triangle_strip) out;
        layout (max_vertices = 4) out;

        out vec2 UV;

        uniform mat4 MVP;
        uniform vec4 up;
        uniform vec4 right;
        uniform float size;

        void main() {
            gl_Position = MVP * (gl_in[0].gl_Position + (-right - up) * size);
            UV = vec2(0.0, 0.0);
            EmitVertex();

            gl_Position = MVP * (gl_in[0].gl_Position + (-right + up) * size);
            UV = vec2(0.0, 1.0);
            EmitVertex();

            gl_Position = MVP * (gl_in[0].gl_Position + (right - up) * size);
            UV = vec2(1.0, 0.0);
            EmitVertex();

            gl_Position = MVP * (gl_in[0].gl_Position + (right + up) * size);
            UV = vec2(1.0, 1.0);
            EmitVertex();
            
            EndPrimitive();
        }
    """

    fragment_shader = """
        # version 450 core
        in vec2 UV;
        out vec4 FragColor;
        uniform sampler2D my_tex;

        void main()
        {
            FragColor = vec4(texture(my_tex, UV).rgb, 1.0);
        }
    """

    def setup(self):
        self.init_time = time_s()

        # loading the shaders, compiling and linking
        V_shader = compileShader(self.vertex_shader, GL_VERTEX_SHADER)
        G_shader = compileShader(self.geometry_shader,
                                 GL_GEOMETRY_SHADER)
        F_shader = compileShader(self.fragment_shader, GL_FRAGMENT_SHADER)
        self.program = compileProgram(
            V_shader, G_shader, F_shader)
        glUseProgram(self.program)

        # uniform attributes in shaders
        self.pos = glGetAttribLocation(self.program, "pos")
        self.mvp = glGetUniformLocation(self.program, 'MVP')
        self.up = glGetUniformLocation(self.program, "up")
        self.right = glGetUniformLocation(self.program, "right")
        self.size = glGetUniformLocation(self.program, "size")
        self.my_tex = glGetUniformLocation(self.program, "my_tex")

        self.time = glGetUniformLocation(self.program, "time")
        self.speed_constant = glGetUniformLocation(
            self.program, "speed_constant")
        self.acc_constant = glGetUniformLocation(self.program, "acc_constant")

        # attributes
        self.vertices = (numpy.random.rand(200*3)-0.5) * 150

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)

        self.vertexBuffer = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self.vertexBuffer)
        vertexData = numpy.array(self.vertices, numpy.float32)
        glBufferData(GL_ARRAY_BUFFER, 4 * len(vertexData),
                     vertexData, GL_DYNAMIC_DRAW)

        # textures

        backgroundImage = Image.open("textures/snow.bmp")
        backgroundImageData = numpy.array(
            list(backgroundImage.getdata()), numpy.uint8)

        self.backgroundTexture = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, self.backgroundTexture)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                     backgroundImage.size[0], backgroundImage.size[1], 0, GL_RGB, GL_UNSIGNED_BYTE, backgroundImageData)

        glBindVertexArray(0)

    def frame(self):
        glClearColor(0.0, 0.0, 0.0, 0.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        up_vec = glm.vec3([0.0, 1.0, 0.0])
        eye_vec = glm.vec3([30.0, 0.0, 30.0])
        center_vec = glm.vec3([0.0, 0.0, 0.0])
        right_vec = glm.normalize(glm.cross(center_vec-eye_vec, up_vec))

        view_mat = glm.lookAt(eye_vec, center_vec, up_vec)
        projection_mat = glm.perspective(
            glm.radians(60.0), 640.0/480.0, 0.1, 100.0)
        model_mat = glm.mat4(1.0)
        MVP_mat: glm.mat4 = projection_mat * view_mat * model_mat

        elapsed = (time_s()-self.init_time)
        size_float = 1.1/(elapsed+1.0)

        # use shader program
        glUseProgram(self.program)
        glBindVertexArray(self.vao)

        # setup uniform values
        glUniformMatrix4fv(self.mvp, 1, GL_FALSE, glm.value_ptr(MVP_mat))
        glUniform4fv(self.up, 1, glm.value_ptr(up_vec))
        glUniform4fv(self.right, 1, glm.value_ptr(right_vec))
        glUniform1fv(self.size, 1, size_float)
        glUniform1i(self.my_tex, 0)

        speed = glm.vec3([1, 0.0, -1])
        acc = glm.vec3([0.0, -2.5, 0.0])

        glUniform3fv(self.speed_constant, 1,
                     glm.value_ptr(speed))
        glUniform3fv(self.acc_constant, 1,
                     glm.value_ptr(acc))

        glUniform1fv(self.time, 1, elapsed)

        # activate texture
        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D, self.backgroundTexture)

        # activate vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, self.vertexBuffer)
        glEnableVertexAttribArray(self.pos)
        glVertexAttribPointer(self.pos, 3, GL_FLOAT, GL_FALSE, 0, None)
        glDrawArrays(GL_POINTS, 0, len(self.vertices)//3)
        glBindVertexArray(0)

        # done, swap
        glutSwapBuffers()

    def mainloop(self):
        glutInitContextVersion(4, 5)
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
        glutInit()
        glutInitWindowSize(640, 480)
        glutCreateWindow("Particles")

        glEnable(GL_DEPTH_TEST)
        glDepthFunc(GL_LESS)

        self.setup()
        glutDisplayFunc(self.frame)
        glutIdleFunc(self.frame)
        glutMainLoop()


ParticleApp().mainloop()
