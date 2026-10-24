"""Bounded source boundary audit; behavioral evidence remains native execution."""
from pathlib import Path
import json,sys
R=Path(__file__).resolve().parents[3]
a=(R/'src/engine/capture/admission.cpp').read_text();n=(R/'src/engine/capture/native.cpp').read_text();p=(R/'src/engine/capture/profile.cpp').read_text()
av=a[a.index('capture_admission::verify_root'):]
nv=n[n.index('native_capture_session::add_root(std::string logical, std::string directory,'):n.index('native_capture_session::add_input')]
pv=p[p.index('profile_capture::bind_root'):p.index('profile_capture::verify()')]
assert 'verify();' in av and 'implementation::kind::supplied_root' in av and 'same_directory(' in av
assert nv.index('admission.verify_root')<nv.index('Capture roots overlap')<nv.index('roots.push_back')
assert 'open_root(directory, true)' in nv and 'open_root(r.host, r.physical_parents)' in n
assert 'segment == ".." && !physical_parents' in n
assert pv.index('_impl->failed = true')<pv.index('session.add_root') and '_impl->admission, slot' in pv
for text in [av,nv,pv]:
 for forbidden in ['::read(', '::pread(', '::readdir(', '::fdopendir(', 'lua_', 'canonical(', 'lexically_normal(']:assert forbidden not in text,forbidden
result=dict(original_supplied_root_only=True,global_receipts_first=True,binding_before_overlap_publication=True,physical_parent_mode_narrow=True,original_private_profile_admission=True,metadata_only_source_boundary=True)
Path(sys.argv[1]).write_text(json.dumps(result,sort_keys=True,indent=2)+'\n')
print('D1 boundary audit pass')
