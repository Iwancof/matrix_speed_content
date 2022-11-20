import re

with open("./settings", "r") as f:
    content = f.read()


settings = {}

for line in content.split('\n'):
    m = re.match("#define (.*) (.*)", line)
    if m:
        settings[m.group(1)] = int(m.group(2), 16)

print(f"loaded settings {settings}")
