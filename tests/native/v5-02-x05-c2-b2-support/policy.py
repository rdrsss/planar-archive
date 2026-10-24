"""Assert the single native global union matches amended policy and all shipped section sources."""
from pathlib import Path
import re,json
R=Path(__file__).resolve().parents[3]
runtime=(R/'src/engine/runtime/runtime.cpp').read_text();block=runtime.split('projection_reserved_ids{',1)[1].split('};',1)[0];native=re.findall(r'"([a-z_]+)"',block)
contract=(R/'docs/vernacular/external-profile-contract.md').read_text().split('Both input kinds reserve the same complete union',1)[1].split('Reservation does not depend',1)[0];declared=re.findall(r'`([a-z_]+)`',contract)
source=(R/'src/stdlib/planar/render.lua').read_text();shipped=set(re.findall(r'section\("([a-z_]+)"',source))|set(re.findall(r'id="(omissions)"',source))
conditional=source.split('for _,id in ipairs({"rationale"',1)[1].split('}) do',1)[0]
shipped|={'rationale'}|set(re.findall(r'"([a-z_]+)"',conditional))
assert len(native)==len(set(native))==18 and native==declared
assert shipped==set(native)-{'recorded_history'},(shipped,set(native)-shipped)
print(json.dumps(dict(global_union=native,shipped_role_conditional_and_omission_ids=sorted(shipped),recorded_history='reserved by amended contract; history projection rendering not delivered in this slice'),indent=2))
