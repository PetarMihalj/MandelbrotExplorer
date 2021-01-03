function getCursorPosition(canvas, event) {
    const rect = canvas.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const y = canvas.height - (event.clientY - rect.top);
    return [x, y];
}

class MandelbrotApp {
    constructor() {
        this.canvas = document.getElementById('canvas');
        this.gl = this.canvas.getContext('webgl2');
        this.center_x = -0.75;
        this.center_y = 0;
        this.size_x = 3;
        this.size_y = this.canvas.height / this.canvas.width * this.size_x;
        // https://math.stackexchange.com/a/2503728
        this.max_iters = 50 * Math.pow(Math.log10(this.canvas.width / this.size_x), 1.25);

        var obj = this;
        this.canvas.addEventListener('click', function (event) {
            config_stack.push([obj.center_x, obj.center_y, obj.size_x, obj.size_y, obj.max_iters]);
            var [x, y] = getCursorPosition(obj.canvas, event);
            var real = (x / obj.canvas.width * 2 - 1) * obj.size_x / 2 + obj.center_x;
            var imag = (y / obj.canvas.height * 2 - 1) * obj.size_y / 2 + obj.center_y;
            obj.center_x = real;
            obj.center_y = imag;
            obj.size_x /= 2;
            obj.size_y /= 2;
            mapp.max_iters = 50 * Math.pow(Math.log10(mapp.canvas.width / mapp.size_x), 1.25);
            obj.create_data_array();
            obj.draw();
        }, false);

        var gl = this.gl;

        this.data_buffer = gl.createBuffer();
        var vertCode = `
            precision highp float;
            attribute vec2 coords;
            attribute vec2 c;

            varying float count;

            precision highp int;
            uniform int max_iters;

            void main(void) {
                vec2 z = vec2(0.0,0.0);
                vec2 z_new = vec2(0.0,0.0);

                for(int i=0;i<1000000;++i){
                    if (i == max_iters){
                        count = float(max_iters);
                        break;
                    }
                    z_new[0] = z[0] * z[0] - z[1] * z[1] + c[0];
                    z_new[1] = z[0] * z[1] * 2.0 + c[1];
                    z = z_new;

                    float abs_val = z_new[0]*z_new[0]+z_new[1]*z_new[1];
                    if (abs_val >= 4.0){
                        count = float(i); 
                        break;
                    }
                }

                gl_Position = vec4(coords, 0.0, 1.0);
            }
        `;
        var vertShader = gl.createShader(gl.VERTEX_SHADER);
        gl.shaderSource(vertShader, vertCode);
        gl.compileShader(vertShader);
        console.log(gl.getShaderInfoLog(vertShader));

        var fragCode = `
            precision highp float;
            precision highp int;
            uniform int max_iters;
            varying float count;

            // https://github.com/hughsk/glsl-hsv2rgb
            vec3 hsv2rgb(vec3 c) {
                vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
                vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
                return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
            }
            
            void main(void) {
                float max_iters_fl = float(max_iters);
                float hue = count / max_iters_fl;
                gl_FragColor = vec4(hsv2rgb(vec3(hue, 0.8, 1.0)), 1.0);
            }
            `;
        var fragShader = gl.createShader(gl.FRAGMENT_SHADER);
        gl.shaderSource(fragShader, fragCode);
        gl.compileShader(fragShader);
        console.log(gl.getShaderInfoLog(fragShader));

        this.shaderProgram = gl.createProgram();
        gl.attachShader(this.shaderProgram, vertShader);
        gl.attachShader(this.shaderProgram, fragShader);
        gl.linkProgram(this.shaderProgram);
        gl.useProgram(this.shaderProgram);
        console.log(gl.getProgramInfoLog(this.shaderProgram));

        gl.enable(gl.DEPTH_TEST);
        gl.viewport(0, 0, this.canvas.width, this.canvas.height);

        this.create_data_array();
        this.draw();
    }

    create_data_array() {
        var center_x = this.center_x;
        var center_y = this.center_y;
        var size_x = this.size_x;
        var size_y = this.size_y;

        this.data = [];
        for (var i = 0; i < this.canvas.width; i++) {
            for (var j = 0; j < this.canvas.height; j++) {
                this.data.push(i / this.canvas.width * 2 - 1);
                this.data.push(j / this.canvas.height * 2 - 1);
                this.data.push((i / this.canvas.width * 2 - 1) * size_x / 2 + center_x);
                this.data.push((j / this.canvas.height * 2 - 1) * size_y / 2 + center_y);
            }
        }
    }

    draw() {
        var gl = this.gl;

        gl.bindBuffer(gl.ARRAY_BUFFER, this.data_buffer);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(this.data), gl.STATIC_DRAW);

        var coords = gl.getAttribLocation(this.shaderProgram,"coords");
        gl.vertexAttribPointer(coords, 2, gl.FLOAT, false, 4 * 4, 0);
        gl.enableVertexAttribArray(coords);


        var max_iters_loc = gl.getUniformLocation(this.shaderProgram, "max_iters");
        gl.uniform1i(max_iters_loc, this.max_iters);

        var c = gl.getAttribLocation(this.shaderProgram,"c");
        gl.vertexAttribPointer(c, 2, gl.FLOAT, false, 4 * 4, 8);
        gl.enableVertexAttribArray(c);

        gl.clearColor(0, 0, 0, 1);
        gl.clear(gl.COLOR_BUFFER_BIT);
        gl.drawArrays(gl.POINTS, 0, this.data.length / 4);
    }
}

mapp = new MandelbrotApp();
config_stack = []

document.addEventListener('keydown', function (event) {
    if (event.code == 'Space' && config_stack.length > 0) {
        [mapp.center_x, mapp.center_y, mapp.size_x, mapp.size_y, mapp.max_iters] = config_stack.pop();
        mapp.create_data_array();
        mapp.draw();
    }
    if (event.code == 'Escape') {
        config_stack = []
        mapp.center_x = -0.75;
        mapp.center_y = 0;
        mapp.size_x = 3;
        mapp.size_y = mapp.canvas.height / mapp.canvas.width * mapp.size_x;
        mapp.max_iters = 50 * Math.pow(Math.log10(mapp.canvas.width / mapp.size_x), 1.25);
        mapp.create_data_array();
        mapp.draw();
    }

});
