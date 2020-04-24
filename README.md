# `"emo"`

![workflow](https://github.com/PsiACE/emo/workflows/ci_meson/badge.svg)
![quality](https://www.code-inspector.com/project/6258/status/svg)

[English](./README.md) | [中文版](./README_zh.md)

> Emotions allow us to express ourselves better.

The `"emo"` programming language is dedicated to exploring simple yet expressive methods, powerful and easy to use.

 _**WIP**_ This project is still in the stage of personal development and research, and only accepts the submission of optimization and patches, but any discussion on design and features is welcome.

If you are familiar with any C-like programming language, you can easily understand the syntax rules of `"emo"`. Of course, `"emo"` also draws some inspiration from other more modern programming languages to improve readability and programming experience.

## Usage

If you want to try it, consider installing the `meson` build system, and `ninja` must also be installed with it.
For example, on the fedora platform, run `dnf install meson`.

```bash
git clone git@github.com:PsiACE/emo.git # or https://github.com/psiace/emo.git
cd emo
meson setup build
# Just for compiler optimization. Otherwise it will be very slow.
meson configure build -Dc_args="-O3" 
meson install -C build
```

Now it should be added to your system. Run `emo` in the terminal or check the documentation.

## Contact

Chojan Shang - [@PsiACE](https://github.com/psiace) - <psiace@outlook.com>

Project Link: [https://github.com/psiace/emo](https://github.com/psiace/emo)

## License

This project is licensed under the terms of the [MIT license](./LICENSE).

## Credits

- [Crafting Interpreters](http://www.craftinginterpreters.com): A handbook for making programming languages. A lot of code for `"emo"` comes directly or indirectly from here.
