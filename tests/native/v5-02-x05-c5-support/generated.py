from freeze import *
config=sys.argv[1];ROOT=R;build=R/('build/rewrite-'+config);dest=E/config
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
variants={}
for manifest in (build/'src/engine').glob('*-generated/generation.json'):
    if manifest.parent.name=='v5-02-x05-a0-generated':continue
    data=json.loads(manifest.read_text());data=data.get('files',data)
    for name,row in data.items():
        original=ROOT/'src/engine'/('export' if name.startswith('source_export') else 'ledger')/name;generated=manifest.parent/name
        assert sha(original)==row['source_sha256'] and sha(generated)==row['generated_sha256'] and row['reverse_identical']
        text=original.read_text()
        for sub in row['substitutions']:
            assert text.count(sub['before'])==sub['count'];text=text.replace(sub['before'],sub['after'])
        assert text==generated.read_text()
        for sub in reversed(row['substitutions']):text=text.replace(sub['after'],sub['before'])
        assert text==original.read_text()
    variants[manifest.parent.name]=data
assert len(variants)==5;write(dest/'five-storage-export-variants.json',variants)
generated=build/'src/engine/v5-02-x05-a0-generated';generation=json.loads((generated/'generation.json').read_text())
for name,row in generation.items():
 if name=='lua':continue
 original=ROOT/'src/engine/runtime'/('runtime.cppm' if name.endswith('.cppm') else 'runtime.cpp');assert sha(original)==row['source_sha256'] and sha(generated/name)==row['generated_sha256'] and row['reverse_identical'];text=original.read_text()
 for sub in row['substitutions']:assert text.count(sub['before'])==sub['count'];text=text.replace(sub['before'],sub['after'])
 assert text==(generated/name).read_text()
 for sub in reversed(row['substitutions']):assert text.count(sub['after'])==sub['count'];text=text.replace(sub['after'],sub['before'])
 assert text==original.read_text()
assert (generated/'commands_0.lua').read_bytes()==(ROOT/'src/stdlib/planar/commands.lua').read_bytes();write(dest/'a0-runtime-audit.json',generation)

cc=json.loads((build/'compile_commands.json').read_text());identity=[]
for row in cc:
 if '-generated/' in row['file']:
  p=Path(row['file']);assert p.is_relative_to(build);assert config=='debug' or 'rewrite-debug' not in row['command']
  identity.append({'file':str(p),'source':entry(p),'command':row['command'],'directory':row['directory']})
assert identity
write(dest/'generated-compile-identity.json',identity)
print('five storage/export and actual runtime/interface reversals plus generated compile identities pass')
