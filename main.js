function getCursorPosition(canvas, event) {
    const rect = canvas.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const y = canvas.height - (event.clientY - rect.top);
    return [x, y];
}

class MandelbrotApp {
    constructor() {
        this.canvas = document.getElementById('my_Canvas');
        this.gl = this.canvas.getContext('webgl2');
        this.center_x = -0.75;
        this.center_y = 0;
        this.size_x = 3;
        this.size_y = this.canvas.height / this.canvas.width * this.size_x;

        var obj = this;
        this.canvas.addEventListener('click', function (event) {
            config_stack.push([obj.center_x, obj.center_y, obj.size_x, obj.size_y]);
            var [x, y] = getCursorPosition(obj.canvas, event);
            var real = (x / obj.canvas.width * 2 - 1) * obj.size_x / 2 + obj.center_x;
            var imag = (y / obj.canvas.height * 2 - 1) * obj.size_y / 2 + obj.center_y;
            obj.center_x = real;
            obj.center_y = imag;
            obj.size_x /= 2;
            obj.size_y /= 2;
            obj.create_data_array();
            obj.draw();
        }, false);

        var gl = this.gl;

        this.data_buffer = gl.createBuffer();
        var vertCode = `
            attribute vec2 coords;
            attribute vec2 c;

            varying vec2 final_z;

            void main(void) {
                vec2 z = vec2(0.0,0.0);
                vec2 z_new = vec2(0.0,0.0);

                for(int i=0;i<200;++i){
                    z_new[0] = z[0] * z[0] - z[1] * z[1] + c[0];
                    z_new[1] = z[0] * z[1] * 2.0 + c[1];
                    float abs_val = z_new[0]*z_new[0]+z_new[1]*z_new[1];
                    if (abs_val >= 4.0){
                        z_new[0]=2.1;
                        z_new[0]=2.1;
                    }

                    
                    z = z_new;
                }

                gl_Position = vec4(coords, 0.0, 1.0);
                final_z = z;
            }
        `;
        var vertShader = gl.createShader(gl.VERTEX_SHADER);
        gl.shaderSource(vertShader, vertCode);
        gl.compileShader(vertShader);
        console.log(gl.getShaderInfoLog(vertShader));

        var fragCode = `
            precision highp float;
            varying vec2 final_z;
            void main(void) {
                float abs_val_sq = final_z[0]*final_z[0]+final_z[1]*final_z[1];
                float inside = 1.0 - step(4.0, abs_val_sq);

                gl_FragColor = vec4(inside, inside, inside, 1.0);
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

        var coords = 0;
        gl.vertexAttribPointer(coords, 2, gl.FLOAT, false, 4 * 4, 0);
        gl.enableVertexAttribArray(coords);

        var c = 1;
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
        [mapp.center_x, mapp.center_y, mapp.size_x, mapp.size_y] = config_stack.pop();
        mapp.create_data_array();
        mapp.draw();
    }
    if (event.code == 'Escape') {
        config_stack = []
        mapp.center_x = -0.75;
        mapp.center_y = 0;
        mapp.size_x = 3;
        mapp.size_y = mapp.canvas.height / mapp.canvas.width * mapp.size_x;
        mapp.create_data_array();
        mapp.draw();
    }

});