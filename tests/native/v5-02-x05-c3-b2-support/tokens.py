"""Prove comment-only amendments with libclang lexical tokens, preserving literals exactly."""
from pathlib import Path
import ctypes as C,hashlib,json,sys
R=Path(__file__).resolve().parents[3];E=R/'tests/native/v5-02-x05-c3-b2-evidence'
lib=C.CDLL('/opt/homebrew/opt/llvm/lib/libclang.dylib')
class String(C.Structure):_fields_=[('data',C.c_void_p),('flags',C.c_uint)]
class Location(C.Structure):_fields_=[('pointers',C.c_void_p*2),('integer',C.c_uint)]
class Range(C.Structure):_fields_=[('pointers',C.c_void_p*2),('begin',C.c_uint),('end',C.c_uint)]
class Token(C.Structure):_fields_=[('integers',C.c_uint*4),('pointer',C.c_void_p)]
def fn(name,restype,*args):
 f=getattr(lib,name);f.restype=restype;f.argtypes=list(args);return f
index=fn('clang_createIndex',C.c_void_p,C.c_int,C.c_int)(0,0)
parse=fn('clang_parseTranslationUnit',C.c_void_p,C.c_void_p,C.c_char_p,C.POINTER(C.c_char_p),C.c_int,C.c_void_p,C.c_uint,C.c_uint)
getfile=fn('clang_getFile',C.c_void_p,C.c_void_p,C.c_char_p)
location=fn('clang_getLocationForOffset',Location,C.c_void_p,C.c_void_p,C.c_uint)
span=fn('clang_getRange',Range,Location,Location)
lex=fn('clang_tokenize',None,C.c_void_p,Range,C.POINTER(C.POINTER(Token)),C.POINTER(C.c_uint))
kind=fn('clang_getTokenKind',C.c_uint,Token)
spelling=fn('clang_getTokenSpelling',String,C.c_void_p,Token)
text=fn('clang_getCString',C.c_char_p,String)
dispose_string=fn('clang_disposeString',None,String)
def tokens(path):
 path=path.resolve();args=(C.c_char_p*5)(b'-x',b'c++',b'-std=c++26',b'-fno-implicit-modules',b'-fno-implicit-module-maps')
 tu=parse(index,str(path).encode(),args,5,None,0,0);assert tu
 file=getfile(tu,str(path).encode());assert file
 token=C.POINTER(Token)();count=C.c_uint()
 lex(tu,span(location(tu,file,0),location(tu,file,path.stat().st_size)),C.byref(token),C.byref(count))
 result=[]
 for i in range(count.value):
  k=kind(token[i])
  if k==4:continue
  value=spelling(tu,token[i]);result.append([k,text(value).decode('utf-8')]);dispose_string(value)
 fn('clang_disposeTokens',None,C.c_void_p,C.POINTER(Token),C.c_uint)(tu,token,count)
 fn('clang_disposeTranslationUnit',None,C.c_void_p)(tu)
 return result
before=E/'runtime-before-doxygen.cpp.txt';after=R/'src/engine/runtime/runtime.cpp'
a=tokens(before);b=tokens(after);assert a==b,'executable tokens changed'
output=dict(before_sha256=hashlib.sha256(before.read_bytes()).hexdigest(),after_sha256=hashlib.sha256(after.read_bytes()).hexdigest(),tokens=len(a),token_sha256=hashlib.sha256(json.dumps(a,ensure_ascii=False).encode()).hexdigest(),all_noncomment_tokens_exact=True,method='libclang lexical token kind+spelling, excluding only CXToken_Comment; literals preserved')
(E/'comment-token-equivalence.json').write_text(json.dumps(output,sort_keys=True,indent=2)+'\n')
print(json.dumps(output,sort_keys=True))
