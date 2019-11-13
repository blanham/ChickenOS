//! # ChickenOS - Architecture dependent code
//!
//!

pub mod boot;

mod address;

pub use self::address::*;


/// Each Architecture defines certain functions
/// These should probably be implemented as traits at some point
#[cfg(any(target_arch="x86", target_arch="x86_64"))]
mod x86;
#[cfg(any(target_arch="x86", target_arch="x86_64"))]
pub use self::x86::*;

