fn take(v: &mut Vec<i32>) -> i32 {
    v.push(2);
    
    for i in v {
        println!("{}", i);
    }
    
    v[0]
}

fn main() {
    let _x: i32 = 42;
    
    let mut v = vec![1, 2, 3];
    
    take(&mut v);
    
    println!("Hello, world: {}", v.len());
}
