# 13天记录

## harib10c ~ harib10f性能测试记录

|         | harib10c   | harib10d   | harib10e   | harib10f   |
| ------- | ---------- | ---------- | ---------- | ---------- |
| 1       | 3576998040 | 3579240462 | 3570070396 | 3554973476 |
| 2       | 3573183293 | 3553041248 | 3560693171 | 3567088063 |
| 3       | 3587470736 | 3605274378 | 3560521191 | 3596711887 |
| 4       | 3568099725 | 3588960516 | 3582694504 | 3558735812 |
| 5       | 3572333069 | 3583916014 | 3576340651 | 3569951223 |
| average | 3575616973 | 3582086524 | 3570063983 | 3569492092 |
| diff    | 19371011   | 52233130   | 22173313   | 41738411   |

看起来好像越来越慢，不知道为啥，不纠结这个，性能测试完成。

## harib10g

|         | harib10g  |
| ------- | --------- |
| 1       | 363357333 |
| 2       | 350514195 |
| 3       | 349322069 |
| 4       | 353501864 |
| 5       | 347625371 |
| average | 352864166 |
| diff    | 15731962  |

跳变挺大，性能更低了，不纠结

## 后续

由于本身使用了内核的hlist链表实现，就不需要做harib10h和harib10i了








