from pathlib import Path
import re
import qrcode

header = Path('include/credentials.h')
text = header.read_text(encoding='utf-8')
entries = re.findall(r'\{\s*"(Mouse\d{2})",\s*"([A-Za-z0-9]{8})"\s*\}', text)

out_dir = Path('QRCodes')
out_dir.mkdir(exist_ok=True)

for ssid, password in entries:
    qr = qrcode.QRCode(
        version=2,
        error_correction=qrcode.constants.ERROR_CORRECT_M,
        box_size=10,
        border=4,
    )
    qr.add_data(f'WIFI:S:{ssid};T:WPA;P:{password};;')
    qr.make(fit=True)
    img = qr.make_image(fill_color='black', back_color='white')
    out_path = out_dir / f'{ssid}.png'
    img.save(out_path)
    print(f'Created {out_path}')
