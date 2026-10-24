# R14-B2 reviewer evidence

Independent verification of candidate
704c8c5f514d6159b6351ac169207f868e009bc36d8de39ab15db8568501de87.
See docs/vernacular/rounds/R14-B2-review.md for verdict, commands, counts and limits.

`build_probes.py` reconstructs disposable reviewer-only probes from `probes.cpp`
using the current candidate's module map and unchanged archives. Run the resulting
`probes '[review]' --reporter console` to select the five owned probes; inclusion of
the candidate scratch fixture also registers its ten original cases, which are
excluded by that filter. All scratch work is per-test and removed after execution.
The VFS and auto-extension instrumentation exists only in this test process;
production source, generated product variants and archives are unchanged.

`probes-final.txt` is the final five-case/1,344-assertion run. `probes.txt` preserves
the first four-case run. Executable/object SHA256 values are in
`probe-products.json`; the disposable build products were removed after review.
`manifest.json` freezes all retained reviewer files and the review record.
