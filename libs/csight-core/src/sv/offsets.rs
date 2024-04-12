use num_enum::IntoPrimitive;

#[derive(Debug, Clone, Copy, Eq, PartialEq, IntoPrimitive)]
#[repr(u64)]
pub enum Offsets {
    MainSingleton = 0x42d0638,
    // v3.0.1
    Party = 0x4763C98,
    MyStatus = 0x47350d8,
}
