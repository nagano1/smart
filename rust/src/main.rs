mod parse_util {
    
    pub fn utf16_length(utf8_chars:&str, byte_limit:usize)-> usize {
        let size_table: [u8; 256] = [
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
            2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
            3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
            4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
        ];

        let mut pos:usize = 0;
        let mut utf16length:usize = 0;

        let bytes = utf8_chars.as_bytes();
        //let bytes_size = bytes.len();

        while pos < byte_limit {
            let idx:usize = bytes[pos] as usize;
            let bytes2 = size_table[idx];
            pos += bytes2 as usize;
            
            utf16length += match bytes2 > 3 {
                true => 2, false => 1
            };
        }

        return utf16length;
    }
}


#[cfg(test)]
mod tests {
    use super::parse_util;

    #[test]
    fn it_works() {
        let x = "👨‍👩‍👧";
        let len = parse_util::utf16_length(x, x.as_bytes().len());
        assert_eq!(len, 8);

        let x = "a𐐀b";
        let len = parse_util::utf16_length(x, x.as_bytes().len());
        assert_eq!(len, 4);
    }

    #[test]
    fn it_works2() {
        let x = "de 13.0 と Emoji 13.0 に準拠した 😀😁😂などの色々な表情の顔文字や 👿悪魔 👹鬼 👺天狗 👽エイリアン 👻おばけ 😺ネコの顔文字と💘❤💓💔💕💖ハ";
        let len = parse_util::utf16_length(x, x.as_bytes().len());
        assert_eq!(len, 96);
    }

    #[test]
    fn it_works3() {
        let x = "我喜欢吃水果。Wǒ xǐhuan chī shuǐguǒ．私は果物が好きです。";
        let len = parse_util::utf16_length(x, x.as_bytes().len());
        assert_eq!(len, 39);

        let x = "안녕하세요";
        let len = parse_util::utf16_length(x, x.as_bytes().len());
        assert_eq!(len, 5);
    }
}



fn main() {
    {
    
    }
}
