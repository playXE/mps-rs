#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(unused_mut)]

pub mod raw {
    include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
}

pub(crate) use raw::*;
pub(crate) use core::mem::MaybeUninit;

#[macro_use]
pub mod macros {
    use super::*;

    #[macro_export]
    macro_rules! mps_key_args_end {
        () => {
           (&_mps_key_ARGS_END as *const _) as *mut _ 
        };
    }
    #[macro_export]
    macro_rules! mps_args_begin {
        ($var : ident) => {
            unsafe {
                let mut $var : [mps_arg_s;MPS_ARGS_MAX as usize] = core::mem::MaybeUninit::zeroed().assume_init();
                paste::item!(let mut [<$var _i>] = 0u32;);
                _mps_args_set_key($var.as_mut_ptr(), paste::item!([<$var _i>]), mps_key_args_end!());
                ($var,paste::item!([<$var _i>]))

            }
        }
    }
    #[macro_export]
    macro_rules! mps_args_add_field {
        ($var : ident,$i : expr,$key: expr,$field: ident,$val: expr) => {
            unsafe {
                _mps_args_set_key($var.as_mut_ptr(),$i,$key.as_bytes().as_ptr().cast());
                $var[$i as usize].val.$field = $val;
                $i += 1;
                _mps_args_set_key($var.as_mut_ptr(),$i,mps_key_args_end!());
            }
        };
    }
    #[macro_export]
    macro_rules! mps_args_add {
        ($var : ident,$i: expr,$key: ident,$val : expr) => {
            let lit = stringify!($var);
            mps_args_add_field!($var,$i,lit,$key,$val);
        };
    }

    #[macro_export]
    macro_rules! mps_args_done {
        ($var : ident,$i: expr) => {
            unsafe {
                _mps_args_set_key($var.as_mut_ptr(),$i,mps_key_args_end!());
                $i = MPS_ARGS_MAX;
            }
        }
    }
    
}


pub const fn mps_message_type_finalization() -> u32 {
    _mps_MESSAGE_TYPE_FINALIZATION
}

pub const fn mps_message_type_gc() -> u32 {
    _mps_MESSAGE_TYPE_GC
}

pub const fn mps_message_type_gc_start() -> u32 {
    _mps_MESSAGE_TYPE_GC_START
}

pub fn mps_rank_ambig() -> raw::mps_rank_t {
    unsafe {raw::mps_rank_ambig()}
}

pub fn mps_rank_exact() -> raw::mps_rank_t {
    unsafe {raw::mps_rank_exact()}
}
pub fn mps_rank_weak() -> raw::mps_rank_t {
    unsafe {raw::mps_rank_weak()}
}

pub const MPS_RM_CONST: raw::mps_rm_t = 1 << 0;
pub const MPS_RM_PROT: raw::mps_rm_t = 1 << 1;
pub const MPS_RM_PROT_INNER: raw::mps_rm_t = 1 << 1;


pub struct MpsAllocationPoint {
    raw: raw::mps_ap_t,
}

pub struct MpsPool {
    raw: raw::mps_pool_t
}

impl MpsAllocationPoint {
    pub fn create(pool: &MpsPool,args: &[raw::mps_arg_s]) -> MpsAllocationPoint {
        unsafe {
            let mut ap = core::mem::MaybeUninit::zeroed().assume_init();
            raw::mps_ap_create_k(&mut ap,pool.raw,args.as_ptr() as *mut _);
            Self {
                raw: ap
            }
        }
    }   
}

impl Drop for MpsAllocationPoint {
    fn drop(&mut self) {
        unsafe {
            raw::mps_ap_destroy(self.raw);
        }
    }
}