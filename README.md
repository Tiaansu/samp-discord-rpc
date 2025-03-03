# SA:MP/open.mp Discord Rich Presence plugin

This plugin features custom server logos

## How to add your own servers with logos:
1. Fork the repository
2. Create `ip_port.json` (e.g. `127.0.0.1_7777.json`) file in `servers` folder
> [!IMPORTANT]
> For some reason, IP address are only allowed.
> If you know how can this plugin, please open a pull request.
3. Fill-up this (all fields are optional)
```json
{
    "application_id": "",
    "logo": "",
    "small_image": "",
    "large_text": "",
    "small_text": "",
    "details": "",
    "state": "",
    "buttons": []
}
```

button structure:
```json
{
    "label": "",
    "url": ""
}
```

example:
```jsonc
{
    // ... other config
    "buttons": [
        {
            "label": "",
            "url": ""
        }
    ]
}
```
4. Create pull request
5. Wait to be accepted.

## Placeholders

This plugin also has support for placeholders.

Current placeholders:

- SERVER_NAME
- SERVER_IP
- SERVER_PORT
- USERNAME
- PLAYERS
- MAX_PLAYERS
- LANGUAGE
- GAMEMODE

### Example usage of placeholders

Playing in {SERVER_NAME}.

## Preview
![Screenshot](./assets//preview-1.png)
![Screenshot](./assets//preview-2.png)
![Screenshot](./assets//preview-3.png)

## Special thanks to:
- Hual - for creating [samp-discord-plugin](https://github.com/Hual/samp-discord-plugin)
- open.mp team - the [query.cpp](./src/query.cpp) is from one of their projects, I just asked AI to convert it to C++ from Rust.
