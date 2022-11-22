import re

with open("./settings.h", "r") as f:
    content = f.read()


settings = {}

for line in content.split('\n'):
    if line.startswith('//'):
        print(f'found ignore config. {line}')
        continue
    m = re.match("#define (.*?) (.*)", line)
    if m:
        try:
            val = int(m.group(2), 0x10)
        except:
            try:
                val = m.group(2)
            except:
                val = None

        settings[m.group(1)] = val

# print(f"loaded settings {settings}")
print("loaded settings")
for key, val in settings.items():
    # print(f"{key} = {val}")
    pass
