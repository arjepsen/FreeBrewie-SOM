# Brewie SOM Service Autostart
_Date: 2026-06-25_

## Purpose
This note captures the transition from manual BrewieApp bring-up to systemd-managed startup on the SOM.

Initial observed state:

```text
ExecStart=/opt/brewie/hello.sh
```

Current proven state:

```text
ExecStart=/opt/brewie/brewie_app
```

`brewie.service` is enabled and has been observed running `brewie_app` as the main process.
The target screen comes on through the service-started app, and the journal shows heartbeat
transmit plus `STATUS_REPORT` receive.

## Current proven manual baseline
The real BrewieApp has been manually installed at:

```text
/opt/brewie/brewie_app
```

The app has been manually run as:

```bash
sudo -u brewie /opt/brewie/brewie_app
```

Current proven behavior:

- the screen shows the current status view
- status information keeps updating
- the `brewie` user has access to `/dev/ttyS1`
- the `brewie` user has access to `/dev/dri/card0`

This manual baseline was enough to test service startup. Service startup is now proven too.

## Install or refresh tracked service file on the SOM
From the development VM, copy the tracked service file to the SOM:

```bash
scp Deploy/Systemd/brewie.service admin@<som-ip>:/home/admin/brewie.service
```

On the SOM:

```bash
sudo cp /etc/systemd/system/brewie.service /etc/systemd/system/brewie.service.bak
sudo install -o root -g root -m 0644 /home/admin/brewie.service /etc/systemd/system/brewie.service
sudo systemctl daemon-reload
sudo systemctl restart brewie.service
```

## Inspect service result
```bash
sudo systemctl status brewie.service --no-pager
sudo journalctl -u brewie.service -n 100 --no-pager
```

Expected/current proven result:

- `Main PID` should be `brewie_app`, not `hello.sh`
- the screen should show the current status view without manually launching the app
- the journal should show no repeated restart loop

## Roll back to previous service
If the service startup test fails:

```bash
sudo cp /etc/systemd/system/brewie.service.bak /etc/systemd/system/brewie.service
sudo systemctl daemon-reload
sudo systemctl restart brewie.service
```

## Notes
The current screen is better described as an early status screen than a boot screen. A true boot screen would be a short transitional display while the appliance starts, then it would hand off to a status/home/fault flow.
