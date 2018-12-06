fn take(v: Vec<i32>) {

}

fn main() {
    let w;

    {
        let v = vec![1, 2, 3];
        w = &v;
    }


    println!("Hello, world: {}", w.len());
}
