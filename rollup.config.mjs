import typescript from "@rollup/plugin-typescript";
import nodeResolve from "@rollup/plugin-node-resolve";
import babel from "@rollup/plugin-babel";
import commonjs from "@rollup/plugin-commonjs";
import json from "@rollup/plugin-json";
import compiler from "@ampproject/rollup-plugin-closure-compiler";

export default {
  input: "src/ts/index.ts",
  output: {
    file: "src/pkjs/index.js",
    format: "iife",
    compact: true,
  },
  plugins: [
    json(),
    typescript(),
    commonjs(),
    nodeResolve(),
    babel({ babelHelpers: "inline", extensions: [".ts", ".js"] }),
    compiler({
      compilation_level: "SIMPLE_OPTIMIZATIONS",
      language_in: "ECMASCRIPT_2015",
      language_out: "ECMASCRIPT5",
    }),
  ],
};
