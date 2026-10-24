"""Original retained authority source boundary; behavioral proof is separate."""
from pathlib import Path
import json,sys,re
R=Path(__file__).resolve().parents[3]
a=(R/'src/engine/capture/admission.cpp').read_text();n=(R/'src/engine/capture/native.cpp').read_text();p=(R/'src/engine/capture/profile.cpp').read_text()
i=(R/'src/engine/capture/admission.cppm').read_text()
assert 'std::shared_ptr<implementation> _impl' in i and 'std::shared_ptr<capture_admission::implementation> _state' in i
assert 'std::size_t _index' in i or 'std::size_t                                     _index' in i or '_index;' in i
retain=a[a.index('capture_admission::retain_file'):a.index('retained_file_receipt::retained_file_receipt')]
assert 'implementation::kind::supplied_file' in retain and '_impl->check()' in retain
assert 'sizeof(retained_file_receipt) + sizeof(std::optional<retained_file_receipt>)' in retain
read=n[n.index('  retained read_file('):n.index('  void retain(')]
assert read.index('authority->verify(f.fd)')<read.index('std::string data(')<read.index('::read(')
assert 'original.same_selection(*authority)' in read and 'admission->poison()' in read
assert 'std::optional<retained_file_receipt> authority' in n or 'authority;' in n
assert 'fresh.authority.emplace(std::move(receipt))' in n
assert 'Retained file authority cannot be replaced' in n
assert 'implementation::operation guard' in n and re.search(r'operation\s+guard\{this\}',n)
assert '_impl->admission.retain_file(slot)' in p and 'std::move(*token)' in p
snapshot=p[p.index('profile_capture::snapshot()'):p.index('profile_capture::admit_root')]
assert '.healthy()' in snapshot and 'verify(' not in snapshot
for text in [a,n,p]:
 for forbidden in ['lexically_normal(', 'lua_']: assert forbidden not in text
result=dict(shared_original_state=True,immutable_selection=True,supplied_files_only=True,actual_fd_before_content=True,stored_authority_on_rereads=True,no_cache_authority_replacement=True,no_filesystem_terminal_observer=True,private_profile_forwarding=True)
Path(sys.argv[1]).write_text(json.dumps(result,sort_keys=True,indent=2)+'\n')
print('D2 A source boundary audit pass')
