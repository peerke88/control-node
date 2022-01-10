import os, struct

with open('resources.h', 'w') as fh:
    fh.write('#include <string>\n\n')
    fh.write('namespace Resources\n{\n')
    for fname in os.listdir('resources'):
        with open(os.path.join('resources', fname), 'rb') as fh2:
            binary = fh2.read()
        varName = os.path.splitext(fname)[0]
        fh.write('\tstatic const unsigned char _' + varName + '[] = { ' + ', '.join(str(x) for x in struct.unpack('%sB' % len(binary), binary)) + ' };\n')
        fh.write('\tstatic const std::string ' + varName + '((char*)&(_' + varName + '[0]), ' + str(len(binary)) + ');\n')
    fh.write('}\n')
