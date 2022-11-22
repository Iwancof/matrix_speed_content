import re

def die(msg):
    print(f'found unknown instruction {msg}')
    quit(0)

def left_index(x, y):
    return y * 16 + x

def load_left_fragment(args):
    assert len(args) == 3
    frag_reg = args[0]
    data = args[1].strip()
    y = int(args[2])

    ret = ""
    for i in range(4):
        ret += f'  vmovapd {left_index(i * 8, y)}(@({data})), @({frag_reg}{i})\n'

    return ret

builtins = {
        "LOAD_LEFT_FRAGMENT": load_left_fragment
        }

with open("./block_mult_template.s", "r") as f:
    content = f.read()

aliases = {}

flag = True
while flag:
    flag = False
    code = ""
    for line in content.split('\n'):
        if line.startswith('//'):
            continue
        if '@' in line:
            flag = True
            # found preprocess instruction

            if 'alias' in line:
                m = re.match('.*?@alias (.*?) = (.*?);', line)
                if not m:
                    die(line)

                f = m.group(1)
                t = m.group(2)

                aliases[f] = t

                continue

            if 'builtin_func' in line:
                m = re.match('.*?@builtin_func\\((.*?)\\)\\((.*?)\\)', line)
                if not m:
                    die(line)

                func = m.group(1)
                args = m.group(2).split(',')

                code += builtins[func](args)

                continue

            while True:
                s = re.search('@\\(.*?\\)', line)
                if not s:
                    break
                left, right = s.span()
                f = line[left: right]
                t = aliases[f[2: len(f) - 1]]
                line = line.replace(f, t)

        code += line + '\n'
    content = code

with open("./block_mult.s", "w") as f:
    f.write(code)

