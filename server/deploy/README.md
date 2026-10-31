# Выделенный сервер Vangers: деплой на VPS (без Docker)

Сервер: процесс `vangers_server`, графики нет, ассеты игры не нужны. Порт игры — **TCP 2197**.
Браузерная версия (websockify, 2198) не настроена; при необходимости добавляется отдельно.
На диск пишет только `Rating.lst` и `VangersServer.log` (рабочая папка сервиса `/var/lib/vangers-server`).

## Файлы
| Файл | Назначение |
|---|---|
| `CMakeLists.txt` | сборка только сервера: нужны SDL2, SDL2_net, zlib (Clunk, FFmpeg, Vorbis не нужны) |
| `install.sh` | сборка из этого checkout + установка + systemd. Он же обновляет |
| `update.sh` | `git pull --ff-only` + `install.sh` |
| `vangers-server.service` | юнит systemd (автозапуск, перезапуск при падении) |
| `.env.example` | порт и ключи запуска; копируется в `/etc/vangers-server.env` |
| `backup.sh`, `logrotate.conf` | ежедневная копия `Rating.lst` (cron 04:17), ротация лога |
| `run-local.sh` | локальная сборка и запуск в консоли |

## 1. Локальная проверка (до VPS)
Docker не нужен. Варианты:
- **WSL (Ubuntu)**: `sudo apt install build-essential cmake pkg-config zlib1g-dev libsdl2-dev libsdl2-net-dev`, затем `./server/deploy/run-local.sh`.
- **MSYS2 MINGW64** (обычный MSYS2, не devkitPro-оболочка): `pacman -S --needed mingw-w64-x86_64-{gcc,cmake,ninja,SDL2,SDL2_net,zlib}`, затем `./server/deploy/run-local.sh`.

В консоли должны появиться строки `Multiplayer VANGERS Server ...` и `Main TCP/IP port: 2197`.
Остановка по Ctrl+C сохраняет `Rating.lst` в `server-local-run/`.

Проверка ПК-клиентом: запустите обычный клиент Vangers с ключами `-server 127.0.0.1 -port 2197` (те же ключи использует автоподключение в Switch-порте) и создайте игру.

## 2. Выкладка на VPS (Ubuntu 22.04/24.04, Debian 12)
Нужен белый IPv4 и доступ по SSH. Хватит 1 vCPU / 512 МБ.

```bash
git clone -b switch https://github.com/webvlad/VangersNSX.git vangers   # приватный репозиторий: deploy key или git clone по SSH
cd vangers
sudo ./server/deploy/install.sh
sudoedit /etc/vangers-server.env        # при желании сменить порт; потом: sudo systemctl restart vangers-server
```
Если репозиторий приватный и ставить ключи на VPS не хочется: на ПК `git archive --format=tar.gz -o vangers.tar.gz HEAD`,
`scp vangers.tar.gz user@vps:` и распаковать там. Сервер собирается из того, что лежит в checkout, поэтому изменения в
`server/` и `src/xsocket.cpp` должны быть закоммичены (или лежать в архиве).

### Порты и firewall
Открыть только **TCP 2197** (или ваш `VANGERS_PORT`). UDP-broadcast (`/b-`) в интернете бесполезен и отключён.
```bash
sudo ufw allow OpenSSH && sudo ufw allow 2197/tcp && sudo ufw enable
```
Если у провайдера есть свой firewall (security group), правило нужно добавить и там.
Проверка: на VPS `ss -ltnp | grep 2197`; с вашего ПК (PowerShell) `Test-NetConnection <ip-vps> -Port 2197`.

### Эксплуатация
```bash
systemctl status vangers-server          # состояние
journalctl -u vangers-server -f          # консольный вывод
sudo systemctl restart vangers-server    # перезапуск (Rating.lst сохраняется при штатной остановке)
sudo ./server/deploy/update.sh           # обновление из git, пересборка, перезапуск
```
Бэкап: `/var/backups/vangers-server/Rating-*.lst`, хранится `BACKUP_KEEP` копий (по умолчанию 14). Восстановление:
`sudo systemctl stop vangers-server && sudo cp -p <копия> /var/lib/vangers-server/Rating.lst && sudo chown vangers:vangers /var/lib/vangers-server/Rating.lst && sudo systemctl start vangers-server`.

## Важные особенности
- Сервер записывает `Rating.lst` **только при штатном завершении**. Для этого в `server/kdsplus.cpp` добавлен обработчик SIGINT/SIGTERM (иначе `systemctl stop` убивал бы процесс без сохранения). При аварийном падении или `kill -9` рейтинг с прошлого запуска теряется. Если это станет проблемой, добавим периодическое сохранение.
- Секретов нет: у сервера нет паролей и ключей, в репозиторий кладётся только `.env.example`. IP VPS в файлы не записывается.
- Старые `server/Dockerfile`, `docker-compose.yml`, `docker-cmd.sh` не тронуты и не используются (Docker вы пока не ставите). Из замеченного: Dockerfile клонирует upstream KranX/Vangers и не видит форк; docker-cmd.sh вызывает `./bin/server`, что с Dockerfile совпадает, так что расхождения с `server2127` в файле нет.
