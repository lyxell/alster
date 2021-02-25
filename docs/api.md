# Lua API

## config

### config.bind(sequence, callback)

### config.bind_parameterized(sequence, callback)

```
config.bind_parameterized('h', function(steps)
    buffer.move_left(steps)
end)
```

### config.set(string, value)

### config.get(string)

## buffer

### buffer.move_left(steps)
