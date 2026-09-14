#include "base64.h"
#include <cassert>
#include <vector>
#include <cstdio>
extern "C" size_t base64_scalar_encode(const unsigned char*,size_t,char*);
extern "C" size_t base64_scalar_decode(const unsigned char*,size_t,unsigned char*);
int main(){
 for(size_t n=0;n<=4096;++n){
  std::vector<unsigned char> in(n+1);for(size_t i=0;i<n;++i)in[i]=(i*173+n*37)&255;
  size_t m=base64_encoded_size(n);
  std::vector<char> a(m?m:1),b(m?m:1);
  assert(base64_encode(in.data(),n,a.data())==m);
  assert(base64_scalar_encode(in.data(),n,b.data())==m);assert(a==b);
  std::vector<unsigned char> out(n?n:1);
  assert(base64_decode((unsigned char*)a.data(),m,out.data())==n);
  for(size_t i=0;i<n;++i)assert(out[i]==in[i]);
  assert(base64_decode_unchecked((unsigned char*)a.data(),m,out.data())==n);
  for(size_t i=0;i<n;++i)assert(out[i]==in[i]);
  assert(base64url_encode(in.data(),n,a.data())==m);
  for(size_t i=0;i<m;++i){if(b[i]=='+')b[i]='-';if(b[i]=='/')b[i]='_';}assert(a==b);
 }
 std::vector<unsigned char> invalid(256,'A'),out(192);
 for(size_t i=0;i<invalid.size();++i){invalid[i]='!';assert(base64_decode(invalid.data(),invalid.size(),out.data())==BASE64_ERROR);invalid[i]='A';}
 std::puts("Lengths 0..4096, exact output buffers, URL encoding, unchecked decoding, and invalid characters passed.");
}
