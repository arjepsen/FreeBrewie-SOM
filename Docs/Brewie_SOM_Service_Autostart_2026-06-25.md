# Brewie SOM Service Autostart
_Date: 2026-06-25_

## Purpose
This note captures the current transition from manual BrewieApp bring-up to systemd-managed startup on the SOM.

The service already exists on the SOM and is enabled, but the observed service still launches the old placeholder script:

```text
ExecStart=/opt/brewie/hello.sh
```

The next service test is therefore not creating autostart from scratch. It is replacing the placeholder command with the real BrewieApp command:

```text
ExecStart=/opt/brewie/brewie_app
```

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

This is enough to test service startup.

## Install tracked service file on the SOM
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

Expected result:

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
