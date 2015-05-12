ENTRY (main)

MEMORY
{
ram : ORIGIN = 0x00000000, LENGTH = 32M
rom : ORIGIN = 0x60000000, LENGTH = 8M
}

SECTIONS
{
data : /* Initialized data. */
{
_DataStart = . ;
*(.data)
_DataEnd = . ;
} >ram

data.rel.local :
{
_LocalStart = . ;
*(.data.rel.local)
_LocalEnd = . ;
} >ram

bss : /* Uninitialized data. */
{
_BssStart = . ;
*(.bss)
_BssEnd = . ;
} >ram

text : /* The actual instructions. */
{
*(.text)
*(.got)
*(.got.plt)
*(.rodata)
*(.glue_7)
*(.glue_7t)
} >ram
}
