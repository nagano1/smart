mod hash_util {
    struct GenericBook<T> {
        book: u32,
        seal: Vec<T>
    }
    impl<T> GenericBook<T> {
        pub fn new() -> Self {
            return GenericBook {book: 11, seal: Vec::new()};
        }
        pub fn seal(&mut self, t:T) {
            self.seal.push(t);
        }
        pub fn show(&self, t:T) {
            // self.seal.push(t);
        }
    }

    pub fn calc_hash(key:&str, key_length:usize, max:usize) -> usize {
        let mut sum:usize = key_length;
        let border: usize = 128;

        let mut salt:isize  = 0; // prevent same result from only order different of letters
        let mut i:usize = 0;
        let bytes = key.as_bytes();
        while i < key_length && i < border {
            let unsigned_value: u8 = bytes[i];
            sum += unsigned_value as usize;
            if i % 2 == 0 {
                salt += (unsigned_value as isize) * (i as isize);
            } else {
                salt += ((-1 * (unsigned_value as isize)) * i as isize) as isize;
            }
            i += 1;
        }

        let mut i = key_length-1;
        let mut j = 0;
        while i >= border && j < border {
            let unsigned_value = bytes[i];
            sum += unsigned_value as usize;
            if j % 2 == 0 {
                salt += unsigned_value as isize * j as isize;
            } else {
                salt += (-(unsigned_value as isize)) * j as isize;
            }

            i -= 1;
            j += 1;
        }

        if salt < 0 {
            salt = -salt;
        }
        return (sum + salt as usize ) % max;
    }

}

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

    pub fn is_identifier_letter(ch:u8) -> bool {
        if 'A' as u8 <= ch && ch <= 'Z' as u8 {
            return true;
        }
        else if 'a' as u8 <= ch && ch <= 'z' as u8 {
            return true;
        }
        else if '0' as u8 <= ch && ch <= '9' as u8 {
            return true;
        }
        else if '_' as u8 == ch {
            return true;
        }

        if ch == "　".as_bytes()[0] { // big space
            return false;
        }

        return (ch & 0x80) == 0x80;
    }
}


#[cfg(test)]
mod tests {
    use super::parse_util;
    use super::hash_util;

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


        let res = hash_util::calc_hash("ak", "ak".as_bytes().len(), 10000 as usize);
        let res2 = hash_util::calc_hash("ka", "ka".as_bytes().len(), 10000 as usize);
        assert_eq!(res, 313);
        assert_eq!(res2, 303);
        assert_ne!(res, res2);


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

    #[test]
    fn is_identifier_letter_test() {
        assert_eq!(5-1, 4);

        assert_eq!(true, parse_util::is_identifier_letter('a' as u8));
        assert_eq!(true, parse_util::is_identifier_letter("😂".as_bytes()[0]));
        assert_eq!(true, parse_util::is_identifier_letter("😂".as_bytes()[1]));
        assert_eq!(false, parse_util::is_identifier_letter('\n' as u8));
        assert_eq!(false, parse_util::is_identifier_letter('\t' as u8));
    }
}



fn main() {
    {
    
    }
}
