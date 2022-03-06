% concatenate the 3 base, multi, and ship config data together to form
% a giant cell array of:
% trace MPKIbase MPKImulti MPKIship
% (throw away IPC for now...)
function result = createBaseCellArray(tracenames, base, pol0, pol1, pol2, pol3, pol4)
result = {tracenames{1}, base(1), pol0(1), pol1(1), pol2(1), pol3(1), pol4(1)};
for i = 2:51
    result = cat(1, result, {tracenames{i}, base(i), pol0(i), pol1(i), pol2(i), pol3(i), pol4(i)});
end
end