//index.js
const mxnetAddon = require('bindings')('mxnetaddon');

/**
 * Create a NDArray representation in javascript.
 * @param data Float32Array The data array in the ndarray.
 * @param shape Uint32Array The shape of the array.
 * @return The constructed NDArray object.
 */
function ndarray(data, shape) {
    var data = Float32Array.from(data);
    var shape = Uint32Array.from(shape);
    var size = shape.reduce(function(a, b) { return a * b; }, 1);
    if (data.length != size) {
        throw "Size and shape mismatch";
    }
    return {'data': data, 'shape': shape};
}

module.exports = mxnetAddon;
module.exports.ndarray = ndarray;

console.log('mxnetaddon',mxnetAddon);