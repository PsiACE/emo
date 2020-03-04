# `"emo"`

> Emotions allow us to express ourselves better.

_**WIP**_

The `"emo"` programming language is dedicated to exploring simple yet expressive methods, powerful and easy to use.

## Usage

If you want to try it, consider installing the `meson` build system, and `ninja` must also be installed with it.
For example, on the fedora platform, run `dnf install meson`.

```bash
git clone git@github.com:PsiACE/emo.git # or https://github.com/psiace/emo.git
cd emo
meson builddir
ninja -C builddir # -j8
meson install # for test, just run `./builddir/src/emo`
```

Now it should be added to your system. Run `emo` in the terminal or check the documentation.

## Contact

Chojan Shang - [@PsiACE](https://github.com/psiace) - <psiace@outlook.com>

Project Link: [https://github.com/psiace/emo](https://github.com/psiace/emo)

## License

This project is licensed under the terms of the [MIT license](./LICENSE).

## Credits

- [Crafting Interpreters](http://www.craftinginterpreters.com): A handbook for making programming languages. A lot of code for `"emo"` comes directly or indirectly from here.
