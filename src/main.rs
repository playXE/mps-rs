#[macro_use]
extern crate mps_rs;

use mps_rs::raw::*;

fn main() {
    let (mut args,mut ix) = mps_args_begin!(args);
    mps_args_add!(args,ix,string,"hello".as_bytes().as_ptr().cast());
    mps_args_done!(args,ix);
    mps_args_add!(args,ix,string,"hello".as_bytes().as_ptr().cast());
}