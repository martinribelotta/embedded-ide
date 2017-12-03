# socketwaiter

Execute while cannot connect to host:port

### Purponse

GDB target remote need to connect to gdb server like openocd but when you start both process simultaneous, need to wait for variable time with gdb server is alive.

For this, put in your `.gdbinit`

```
shell socketwaiter localhost:3333
```

Suposing if openocd gdb server run in localhost, port 3333
