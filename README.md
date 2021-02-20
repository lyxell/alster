## Most notable features

* Undo/redo actually gives intuitive buffer positions, i.e.
  undo and then redo should put you back at exactly the same
  position that you started, it should also keep context such
  as selections, especially if you deleted a selection, undoing
  should give you back your selection

* There should be a way to do this movement:

```
int main() {
    if (x == 0) {
        return 20;
                  ↑ cursor here
    }
}
```

jump here, enter insert mode:

```
int main() {
    if (x == 0) {
        return 20;
    }
    
    ↑ cursor here
}
```

To do it in vim, use: "]}o"

## Something that no editor does

```
int main() {}
            ↑ cursor here
```
press enter
```
int main() {
    
    ↑ cursor here
}
```
press backspace
```
int main() {}
            ↑ cursor here
```

Works in vscode, but you need Ctrl+Z instead
of backspace
