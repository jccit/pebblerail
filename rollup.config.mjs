import typescript from "@rollup/plugin-typescript";
import nodeResolve from "@rollup/plugin-node-resolve";
import babel from "@rollup/plugin-babel";
import commonjs from "@rollup/plugin-commonjs";

export default {
  input: "src/ts/index.ts",
  output: {
    file: "src/pkjs/index.js",
    format: "iife",
  },
  plugins: [
    typescript(),
    commonjs(),
    nodeResolve(),
    babel({ babelHelpers: "inline", extensions: [".ts", ".js"] }),
  ],
};
